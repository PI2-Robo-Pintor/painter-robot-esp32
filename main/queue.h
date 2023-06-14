#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Atuadores

QueueHandle_t stepMotorQueue;

QueueHandle_t solenoidQueue;

// Sensores

QueueHandle_t sensorsQueue;
