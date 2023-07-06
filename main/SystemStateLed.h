#pragma once


#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pins.h"

#include "pins.h"
#include "data_command_event.h"
#include "low_high.h"

class SystemStateLed {
  public:
    static const char* tag;
    SystemStateLed();
    void green();
    void yellow();
    void red();
    QueueHandle_t queue;
};
