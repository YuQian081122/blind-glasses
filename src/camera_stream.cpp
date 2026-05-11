/**
 * OV3660 MJPEG 串流實作
 * XIAO ESP32-S3 Sense 腳位
 */

#include "camera_stream.h"
#include "config.h"

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"

// OV3660 DVP：GPIO10 XCLK, 11–18/48 資料線, 13 PCLK, 38 VSYNC, 47 HREF；SCCB 見 config.h CAMERA_SIOD_PIN / CAMERA_SIOC_PIN
#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   10
#define SIOD_GPIO_NUM   CAMERA_SIOD_PIN
#define SIOC_GPIO_NUM   CAMERA_SIOC_PIN
#define Y9_GPIO_NUM     48
#define Y8_GPIO_NUM     11
#define Y7_GPIO_NUM     12
#define Y6_GPIO_NUM     14
#define Y5_GPIO_NUM     16
#define Y4_GPIO_NUM     18
#define Y3_GPIO_NUM     17
#define Y2_GPIO_NUM     15
#define VSYNC_GPIO_NUM  38
#define HREF_GPIO_NUM   47
#define PCLK_GPIO_NUM   13

static httpd_handle_t streamServer = NULL;
static bool streamReady = false;

static esp_err_t streamHandler(httpd_req_t* req) {
  camera_fb_t* fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t* _jpg_buf = NULL;
  char part_buf[128];

  static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace; boundary=frame";
  // 須與伺服器 stream_manager 一致：同一個 multipart 區塊內含 Content-Type + Content-Length，再接 JPEG body
  static const char* _STREAM_BOUNDARY = "\r\n--frame\r\n";

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[CAM] Frame capture failed");
      res = ESP_FAIL;
      break;
    }

    if (fb->format == PIXFORMAT_JPEG) {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    } else {
      bool jpeg_converted = frame2jpg(fb, CAMERA_JPEG_QUALITY, &_jpg_buf, &_jpg_buf_len);
      if (!jpeg_converted) {
        esp_camera_fb_return(fb);
        Serial.println("[CAM] JPEG convert failed");
        res = ESP_FAIL;
        break;
      }
    }

    res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (res != ESP_OK) break;

    size_t hlen = snprintf(
        (char*)part_buf,
        sizeof(part_buf),
        "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
        (uint32_t)(_jpg_buf_len));
    res = httpd_resp_send_chunk(req, (const char*)part_buf, hlen);
    if (res != ESP_OK) break;

    res = httpd_resp_send_chunk(req, (const char*)_jpg_buf, _jpg_buf_len);
    if (res != ESP_OK) break;

    if (fb->format != PIXFORMAT_JPEG && _jpg_buf) {
      free(_jpg_buf);
    }
    esp_camera_fb_return(fb);
    fb = NULL;

    // 讓出 CPU；不再固定延遲 30ms，避免把可用幀率鎖死在 ~33fps
    delay(1);
  }

  if (fb) esp_camera_fb_return(fb);
  return res;
}

static void startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = STREAM_PORT;
  // 控制埠不可與串流埠相同，否則部分 ESP-IDF 版本 httpd_start 會失敗
  config.ctrl_port = STREAM_PORT + 1;
  config.max_open_sockets = 2;
  config.max_uri_handlers = 2;

  if (httpd_start(&streamServer, &config) == ESP_OK) {
    httpd_uri_t stream_uri = {
      .uri = STREAM_PATH,
      .method = HTTP_GET,
      .handler = streamHandler,
      .user_ctx = NULL
    };
    httpd_register_uri_handler(streamServer, &stream_uri);
    Serial.print("[CAM] Stream server on port ");
    Serial.println(STREAM_PORT);
  } else {
    Serial.println("[CAM] Failed to start stream server");
  }
}

bool CameraStream::begin() {
  camera_config_t config = {};
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.ledc_timer = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.pixel_format = PIXFORMAT_JPEG;
#if CAMERA_FRAMESIZE_VGA
  config.frame_size = FRAMESIZE_VGA;  // 640×480，監控較清晰
#else
  config.frame_size = FRAMESIZE_QVGA;  // 320×240，較省頻寬
#endif
  config.jpeg_quality = CAMERA_JPEG_QUALITY;
  config.fb_count = 2;                 // OV3660 建議 2
  config.fb_location = CAMERA_FB_IN_PSRAM;
  // 低延遲模式：優先抓最新幀，避免管線累積造成 1~2 秒延遲
  config.grab_mode = CAMERA_GRAB_LATEST;

  esp_task_wdt_reset();
  esp_err_t err = esp_camera_init(&config);
  esp_task_wdt_reset();
  if (err != ESP_OK) {
    Serial.printf("[CAM] Init failed: 0x%x\n", err);
    return false;
  }

  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
  }

  startStreamServer();
  streamReady = true;
  return true;
}

bool CameraStream::isReady() {
  return streamReady;
}

static camera_fb_t* _pushFb = nullptr;
static uint8_t* _pushConvertedBuf = nullptr;

bool CameraStream::captureJpeg(const uint8_t** outBuf, size_t* outLen) {
  if (!streamReady || !outBuf || !outLen) return false;
  _pushFb = esp_camera_fb_get();
  if (!_pushFb) return false;
  if (_pushFb->format == PIXFORMAT_JPEG) {
    *outBuf = _pushFb->buf;
    *outLen = _pushFb->len;
  } else {
    _pushConvertedBuf = nullptr;
    size_t convertedLen = 0;
    if (!frame2jpg(_pushFb, CAMERA_JPEG_QUALITY, &_pushConvertedBuf, &convertedLen)) {
      esp_camera_fb_return(_pushFb);
      _pushFb = nullptr;
      return false;
    }
    *outBuf = _pushConvertedBuf;
    *outLen = convertedLen;
  }
  return true;
}

void CameraStream::releaseFrame() {
  if (_pushConvertedBuf) {
    free(_pushConvertedBuf);
    _pushConvertedBuf = nullptr;
  }
  if (_pushFb) {
    esp_camera_fb_return(_pushFb);
    _pushFb = nullptr;
  }
}
