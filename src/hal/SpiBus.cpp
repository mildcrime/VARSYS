#include "SpiBus.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace hal {

static SemaphoreHandle_t s_mutex = nullptr;

void spiBusInit() {
    if (!s_mutex) s_mutex = xSemaphoreCreateRecursiveMutex();
}

void spiBusLock() {
    if (s_mutex) xSemaphoreTakeRecursive(s_mutex, portMAX_DELAY);
}

void spiBusUnlock() {
    if (s_mutex) xSemaphoreGiveRecursive(s_mutex);
}

} // namespace hal
