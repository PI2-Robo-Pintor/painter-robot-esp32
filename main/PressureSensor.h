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

#define AMP_MAX_VALUE 167772
#define MAX_PRESSURE 40

class PressureSensor {
  public:
    PressureSensor();
    static void measure_loop(void* args);
    unsigned long convert;
    unsigned long read_count();
    static const char* tag;
    QueueHandle_t queue;
};
