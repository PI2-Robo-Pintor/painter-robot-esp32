#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t StepMotorQueue;

QueueHandle_t SensorsQueue;

#endif // QUEUE_H_INCLUDED