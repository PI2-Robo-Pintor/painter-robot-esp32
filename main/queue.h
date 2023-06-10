#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t stepMotorQueue;

QueueHandle_t sensorsQueue;

#endif // QUEUE_H_INCLUDED