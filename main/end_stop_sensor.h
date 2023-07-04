#pragma once

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "data_command_event.h"
#include "pins.h"

void setup_end_stop_sensor();

static void IRAM_ATTR handle_end_stop(void* args);
