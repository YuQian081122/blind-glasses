/**
 * GPS 資料串流 - NEO-M8N
 * 定期讀取經緯度並 POST 至 /api/gps
 */

#ifndef GPS_STREAM_H
#define GPS_STREAM_H

#include <Arduino.h>
#include <IPAddress.h>

namespace GpsStream {

  void begin();
  void tick();

  bool isReady();
  void setServerIP(IPAddress ip);

}  // namespace GpsStream

#endif  // GPS_STREAM_H
