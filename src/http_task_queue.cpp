#include "http_task_queue.h"
#include "config.h"
#include "server_api.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_heap_caps.h>

enum TaskType : uint8_t {
  TASK_GEMINI = 0,
  TASK_ASR = 1,
};

struct HttpTask {
  TaskType type;
  IPAddress ip;
  char mode[16];
  uint8_t* data;
  size_t len;
};

static QueueHandle_t taskQueue = nullptr;
static volatile bool busy = false;
static volatile bool completedFlag = false;
static volatile unsigned long lastDuration = 0;
static TaskHandle_t workerHandle = nullptr;

static void workerLoop(void* param) {
  (void)param;
  HttpTask task;
  for (;;) {
    if (xQueueReceive(taskQueue, &task, portMAX_DELAY) == pdTRUE) {
      busy = true;
      unsigned long t0 = millis();

      if (task.type == TASK_GEMINI) {
        ServerAPI::requestGemini(task.ip, task.mode);
      } else if (task.type == TASK_ASR) {
        if (task.data && task.len > 0) {
          ServerAPI::uploadAudio(task.ip, task.data, task.len);
          free(task.data);
          task.data = nullptr;
        }
      }

      lastDuration = millis() - t0;
      busy = false;
      completedFlag = true;
    }
  }
}

namespace HttpTaskQueue {

  void begin() {
    if (taskQueue != nullptr) return;
    taskQueue = xQueueCreate(4, sizeof(HttpTask));
    xTaskCreatePinnedToCore(workerLoop, "http_q", 8192, nullptr, 1, &workerHandle, 0);
    Serial.println("[HTTP-Q] Background task queue started");
  }

  void tick() {
    // reserved for future: retry logic, watchdog, etc.
  }

  bool enqueueGemini(IPAddress serverIP, const char* mode) {
    if (!taskQueue) return false;
    HttpTask task = {};
    task.type = TASK_GEMINI;
    task.ip = serverIP;
    strncpy(task.mode, mode ? mode : "general", sizeof(task.mode) - 1);
    task.data = nullptr;
    task.len = 0;
    return xQueueSend(taskQueue, &task, 0) == pdTRUE;
  }

  bool enqueueAsr(IPAddress serverIP, const uint8_t* data, size_t len) {
    if (!taskQueue || !data || len == 0) return false;
    uint8_t* copy = (uint8_t*)heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    if (!copy) copy = (uint8_t*)malloc(len);
    if (!copy) return false;
    memcpy(copy, data, len);

    HttpTask task = {};
    task.type = TASK_ASR;
    task.ip = serverIP;
    task.mode[0] = '\0';
    task.data = copy;
    task.len = len;
    if (xQueueSend(taskQueue, &task, 0) != pdTRUE) {
      free(copy);
      return false;
    }
    return true;
  }

  bool isBusy() {
    return busy;
  }

  bool hasCompletedSinceEnqueue() {
    return completedFlag;
  }

  void clearCompleted() {
    completedFlag = false;
  }

  unsigned long lastTaskDurationMs() {
    return lastDuration;
  }

}  // namespace HttpTaskQueue
