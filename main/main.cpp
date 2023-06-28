#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "StepMotor.h"
#include "data_command.h"
#include "mqtt.h"
#include "queue.h"

static const char* TAG              = "PI2-Robo-Pintor";
static const char* tag_main_control = "Main loop control";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_WS", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    Mqtt mqtt;
    StepMotor motor;

    mqttQueue      = xQueueCreate(10, sizeof(AllData));
    mainQueue      = xQueueCreate(10, sizeof(Command));
    stepMotorQueue = xQueueCreate(10, sizeof(Command));
    sensorsQueue   = xQueueCreate(10, sizeof(AllData));
    solenoidQueue  = xQueueCreate(10, sizeof(unsigned char));
    if (!stepMotorQueue || !sensorsQueue || !solenoidQueue || !mainQueue || !mqttQueue)
        ESP_LOGE(TAG, "Failed to create Queues");
    else {
        motor.queue = stepMotorQueue;

        mqtt.stepMotorQueue = stepMotorQueue;
        mqtt.sensorsQueue   = sensorsQueue;
        mqtt.solenoidQueue  = solenoidQueue;
        mqtt.mainQueue      = mainQueue;
    }

    mqtt.start();

    xTaskCreatePinnedToCore(
        StepMotor::control_loop,
        "Task de controle do motor",
        2048,
        &motor,
        1,
        NULL,
        0);

    BaseType_t result = 0;
    AllData data      = {
             .device = D_STEP_MOTOR,
             .relay  = {
                  .on_off = 1,
                  .id     = RID_COMPRESSOR,
        },
    };

    mqtt.publish(Mqtt::TOPIC_SENSORS, &data);
    while (true) {
        Command command = {
            .type  = (Type)0,
            .value = 0,
        };
        result = xQueueReceive(mainQueue, &command, 1);
        if (result != pdPASS) {
            // ESP_LOGE(tag, "Erro na fila?");
            // continue;
        }

        switch (command.type) {
        case T_MAX_HEIGHT:
            // setar altura máxima do motor
            break;

        case T_MIN_HEIGHT:
            // setar altura mínima do motor
            break;

        case T_ON_OFF:
            // ON
            // ligar motor
            // ligar compressor

            // OFF
            // desligar motor
            // desligar compressor
            break;
        case T_NONE:
            break;

        default:
            ESP_LOGW(tag_main_control, "Command not recognized 0x%02X", command.type);
            break;
        }

        AllData recv_data;
        result = xQueueReceive(sensorsQueue, &recv_data, 1);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
