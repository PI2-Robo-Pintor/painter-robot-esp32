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
#include "data_command_event.h"
#include "mqtt.h"
#include "queue.h"

static const char* TAG              = "PI2-Robo-Pintor";
static const char* tag_main_control = "Main loop control";

// A resposta dessa interrupção depende dos delays no loop principal de controle
static void IRAM_ATTR handle_end_stop(void* args);

void setup_end_stop_sensor() {
    gpio_set_direction(PIN_END_STOP, GPIO_MODE_INPUT);
    gpio_pulldown_en(PIN_END_STOP);
    gpio_set_intr_type(PIN_END_STOP, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_END_STOP, handle_end_stop, NULL);
};

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

    setup_end_stop_sensor();

    Mqtt mqtt;
    StepMotor motor;

    // mainQueue      = xQueueCreate(10, sizeof(Command));
    mainQueue      = xQueueCreate(10, sizeof(EventCommand));
    mqttQueue      = xQueueCreate(10, sizeof(AllData));
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
                  .value = 1,
                  .id    = RID_COMPRESSOR,
        },
    };

    mqtt.publish(Mqtt::TOPIC_SENSORS, &data);
    while (true) {
        Command command = {
            .type  = (Type)0,
            .value = 0,
        };

        EventCommand ec = {0};

        // result = xQueueReceive(mainQueue, &command, 10);
        result = xQueueReceive(mainQueue, &ec, 10);
        if (result != pdPASS) {
            // ESP_LOGE(tag, "Erro na fila?");
            // continue;
        }

        if (ec.type == T_EVENT) {

        } else {
            switch (command.type) {
            case T_NONE:
                break;
            case T_MAX_HEIGHT:
                ESP_LOGW(tag_main_control, "Set MAX height");
                break;

            case T_MIN_HEIGHT:
                ESP_LOGW(tag_main_control, "Set MIN height");
                break;

            case T_ON_OFF:
                if (command.value == ON) {
                    ESP_LOGI(tag_main_control, "Step motor start");
                    // if (motor.state != RUNNING)
                    motor.start();
                    // ligar compressor
                }
                // OFF
                else {
                    ESP_LOGI(tag_main_control, "Step motor stop");
                    motor.stop();
                    // desligar compressor
                }
                break;

            case T_VELOCITY:
                motor.set_speed(command.value);
                break;

            default:
                ESP_LOGW(tag_main_control, "Command not recognized 0x%02X", command.type);
                break;
            }
        }

        AllData recv_data;
        result = xQueueReceive(sensorsQueue, &recv_data, 10);

        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void handle_end_stop(void* args) {
    // Command command = {
    //     .type  = T_ON_OFF,
    //     .value = 0,
    // };

    EventCommand event;
    event.type       = T_EVENT;
    event.event.type = E_REACHED_LOWER_END_STOP_SENSOR;

    xQueueSendFromISR(mainQueue, &event, NULL);
}
