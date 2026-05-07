#ifndef HTTP_TASK_QUEUE_H
#define HTTP_TASK_QUEUE_H

#include <Arduino.h>
#include <IPAddress.h>

namespace HttpTaskQueue {

  void begin();
  void tick();

  bool enqueueGemini(IPAddress serverIP, const char* mode);
  bool enqueueAsr(IPAddress serverIP, const uint8_t* data, size_t len);

  bool isBusy();
  bool hasCompletedSinceEnqueue();
  void clearCompleted();
  unsigned long lastTaskDurationMs();

}  // namespace HttpTaskQueue

#endif  // HTTP_TASK_QUEUE_H
