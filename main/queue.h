#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t mainQueue;
QueueHandle_t mqttQueue;

// Atuadores

QueueHandle_t stepMotorQueue;

QueueHandle_t solenoidQueue;

// Sensores

QueueHandle_t sensorsQueue;
