#pragma once

#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "low_high.h"

typedef enum {
    T_VELOCITY   = 0xC1,
    T_MAX_HEIGHT = 0xC2,
    T_MIN_HEIGHT = 0xC3,
    T_ON_OFF     = 0xC4,
} Type;

class Mqtt {
public:
    QueueHandle_t stepMotorQueue;
    QueueHandle_t solenoidQueue;
    QueueHandle_t sensorsQueue;

    static const char* TAG;
    static const char* TOPIC_STEP_MOTOR;
    static const char* TOPIC_SOLENOID;
    static const char* TOPIC_SENSORS;
    static const char* TOPIC_GENERAL;

    static void handle_event(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
    static void handle_event_data(Mqtt* mqtt, esp_mqtt_event_handle_t event);
    static char parse_command(cJSON* root);

    void start();

private:
    static void log_error_if_nonzero(const char* message, int error_code);
};
