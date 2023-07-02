#pragma once


#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pins.h"

#include "pins.h"
#include "data_command.h"

#define HIGH 1
#define LOW  0


class PressureSensor {
  public:
    PressureSensor();
    static void measure_loop(void* args);
    unsigned long convert;
    unsigned long read_count();
    static const char* tag;
    QueueHandle_t queue;
};
