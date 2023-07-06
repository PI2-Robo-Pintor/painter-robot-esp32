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
#include "end_stop_sensor.h"
#include "low_high.h"
#include "mqtt.h"
#include "queue.h"
#include "relay.h"

static const char* TAG              = "PI2-Robo-Pintor";
static const char* tag_main_control = "Main loop control";

// A resposta dessa interrupção depende dos delays no loop principal de controle

int cm_to_steps(int cm);

void find_initial_position(void* args);

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
    Relay rel(PIN_RELAY_2);

    mainQueue      = xQueueCreate(10, sizeof(EventCommand));
    mqttQueue      = xQueueCreate(10, sizeof(AllData));
    stepMotorQueue = xQueueCreate(10, sizeof(Command));
    sensorsQueue   = xQueueCreate(10, sizeof(AllData));
    solenoidQueue  = xQueueCreate(10, sizeof(unsigned char));
    relayQueue     = xQueueCreate(10, sizeof(unsigned char));

    if (!stepMotorQueue || !sensorsQueue || !solenoidQueue || !mainQueue || !mqttQueue || !relayQueue)
        ESP_LOGE(TAG, "Failed to create Queues");
    else {
        motor.queue = stepMotorQueue;

        mqtt.stepMotorQueue = stepMotorQueue;
        mqtt.sensorsQueue   = sensorsQueue;
        mqtt.solenoidQueue  = solenoidQueue;
        mqtt.mainQueue      = mainQueue;
        rel.queue           = relayQueue;
        mqtt.relayQueue     = relayQueue; // relay mqtt queue
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

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    setup_end_stop_sensor();

    find_initial_position(&motor);

    while (true) {
        EventCommand ec = event_command_reset();

        result = xQueueReceive(mainQueue, &ec, 10);
        if (result != pdPASS) {
            // ESP_LOGE(tag, "Erro na fila?");
            // continue;
        }

        if (ec.type == T_EVENT) {
            switch (ec.event.type) {
            case E_JUST_PRESSED_END_STOP_SENSOR:
                ESP_LOGI(tag_main_control, "PRESSED");
                motor.stop();
                reenable_end_stop_sensor();
                break;
            case E_JUST_RELEASED_END_STOP_SENSOR:
                ESP_LOGI(tag_main_control, "RELEASED");
                motor.stop();
                reenable_end_stop_sensor();
                break;
            case E_REACHED_UPPER_LIMIT:
                ESP_LOGI(tag_main_control, "E_REACHED_UPPER_LIMIT: %d passos", motor.double_the_steps);
                motor.stop();
                break;
            default:
                break;
            }
        } else {
            Command command = ec.command;
            switch (command.type) {
            case T_NONE:
                break;
            case T_MAX_HEIGHT:
                ESP_LOGI(tag_main_control, "Set MAX height %d", command.value);
                motor.set_height_stop(command.value);
                break;

            case T_MIN_HEIGHT:

                ESP_LOGI(tag_main_control, "Set MIN height");
                break;

            case T_ON_OFF:
                if (command.value == ON) {
                    ESP_LOGI(tag_main_control, "Step motor start");
                    motor.start();
                    rel.on();
                }
                // OFF
                else {
                    ESP_LOGI(tag_main_control, "Step motor stop");
                    motor.stop();
                    rel.off();
                }
                break;

            case T_VELOCITY:
                motor.set_delay(command.value);
                break;

            case T_INVERT:
                if (motor.dir_state == D_UP)
                    motor.dir_state = D_DOWN;
                else
                    motor.dir_state = D_UP;

                ESP_LOGI(tag_main_control, "Step motor inverted to %d", motor.dir_state);
                motor.set_direction(motor.dir_state);
                break;

            default:
                ESP_LOGW(tag_main_control, "Command not recognized 0x%02X", command.type);
                break;
            }
        }

        AllData recv_data;
        result = xQueueReceive(sensorsQueue, &recv_data, 10);

        switch (recv_data.device) {
        case D_STEP_MOTOR:
            mqtt.publish(Mqtt::TOPIC_DATA, &recv_data);
            break;
        default:
            break;
        }

        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

// 348'400
int cm_to_steps(int cm) {
    // 1 rev->1600 steps->1cm;
    // 350000/2/1600 = 109.375 cm é a ~distância percorrida da base ao topo
    return cm * StepMotor::STEPS_PER_REVOLUTION;
}

// Vai descendo até encontrar acionar o fim de curso.
void find_initial_position(void* args) {
    const char* tag = "Find HOME";
    ESP_LOGI(tag, "started");

    StepMotor* motor = (StepMotor*)args;
    motor->set_direction(D_DOWN);
    motor->start();
    motor->set_delay(500);

    bool pressed_end_stop  = false;
    bool released_end_stop = false;

    while (true) {
        EventCommand event_command = event_command_reset();
        xQueueReceive(mainQueue, &event_command, portMAX_DELAY);

        ESP_LOGI(tag, "event_command %d", event_command.type);
        ESP_LOGI(tag, "event_command %d", event_command.command.type);
        ESP_LOGI(tag, "event_command %d", event_command.command.value);
        if (event_command.type != T_EVENT) {
            ESP_LOGW(tag, "Should've received an event");
            continue;
        }

        switch (event_command.event.type) {
        case E_JUST_PRESSED_END_STOP_SENSOR:
            reenable_end_stop_sensor();
            ESP_LOGI(tag, "PRESSED end stop");
            pressed_end_stop = true;
            motor->stop();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            motor->set_direction(D_UP);
            motor->start();
            break;

        case E_JUST_RELEASED_END_STOP_SENSOR:
            reenable_end_stop_sensor();
            ESP_LOGI(tag, "RELEASED end stop");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            motor->stop();
            released_end_stop       = true;
            motor->double_the_steps = 0;
            break;

        default:
            ESP_LOGW(tag, "Should've not have received this event: 0x%02X", event_command.event.type);
            break;
        }

        if (pressed_end_stop && released_end_stop) {
            return;
        }
    }
}