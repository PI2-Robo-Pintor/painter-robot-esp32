#pragma once

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "data_command_event.h"
#include "low_high.h"
#include "pins.h"

void setup_end_stop_sensor();

void init_read_end_stop_timer();

void reenable_end_stop_sensor();
