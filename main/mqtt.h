#ifndef MQTT_H_INCLUDED
#define MQTT_H_INCLUDED

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mqtt_client.h"

// #include "queue.h"
extern QueueHandle_t stepMotorQueue;
#include "tag.h"

#define HIGH 1
#define LOW  0

typedef struct
{
    QueueHandle_t queue;
} Mqtt;

void log_error_if_nonzero(const char* message, int error_code);
void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
void mqtt_app_start(Mqtt* mqtt);

#endif // MQTT_H_INCLUDED
