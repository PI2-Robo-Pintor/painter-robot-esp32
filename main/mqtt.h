#pragma once

#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "data_command_event.h"
#include "low_high.h"

class Mqtt {
public:
    QueueHandle_t mainQueue;
    // Essas filas são apenas para depuração
    QueueHandle_t stepMotorQueue;
    QueueHandle_t solenoidQueue;
    QueueHandle_t sensorsQueue;
    QueueHandle_t relayQueue;

    static const char* TAG;
    static const char* TOPIC_STEP_MOTOR;
    static const char* TOPIC_SOLENOID;
    static const char* TOPIC_SENSORS;
    static const char* TOPIC_RELAY;

    static const char* TOPIC_GENERAL;
    static const char* TOPIC_DATA;

    static void handle_event(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
    static void handle_event_data(Mqtt* mqtt, esp_mqtt_event_handle_t event);
    void publish(const char* topic, AllData* sensor_state);

    void start();

private:
    static void log_error_if_nonzero(const char* message, int error_code);
    esp_mqtt_client_handle_t client;
};
