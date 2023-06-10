#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "StepMotor.h"
#include "mqtt.h"
#include "queue.h"
#include "tag.h"

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

    qqueue = xQueueCreate(10, sizeof(unsigned char));
    if (!qqueue)
        ESP_LOGE(TAG, "Failed to create Queue");
    else {
        ESP_LOGI(TAG, "      Queue %p", qqueue);
        motor.queue = qqueue;
        mqtt.queue  = qqueue;
        ESP_LOGI(TAG, "motor.queue %p", motor.queue);
        ESP_LOGI(TAG, "mqtt .queue %p", motor.queue);
    }

    mqtt_app_start(&mqtt);

    ESP_LOGW(TAG, "Addr motor %p. oi=%d", &motor, motor.oi);
    xTaskCreate(
        step_motor_control_loop,
        "Task de controle do motor",
        2048,
        &motor,
        1,
        NULL);
}
