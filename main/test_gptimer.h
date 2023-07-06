#pragma once

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

gptimer_handle_t my_timer = NULL;

static bool IRAM_ATTR timer_handler(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data) {
    BaseType_t high_task_awoken = pdFALSE;

    EventCommand ec = event_command_reset();
    ec.type         = -1;
    xQueueSendFromISR(mainQueue, &ec, NULL);
    return (high_task_awoken == pdTRUE);
}

void init_tmp_timer() {
    gptimer_alarm_config_t alarm_config;
    alarm_config.alarm_count                = 1'000'000;
    alarm_config.reload_count               = 0;
    alarm_config.flags.auto_reload_on_alarm = false;

    gptimer_config_t timer_config;
    timer_config.clk_src       = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction     = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz, 1 tick=1us

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &my_timer));
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_handler,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(my_timer, &cbs, NULL));

    gptimer_set_alarm_action(my_timer, &alarm_config);

    ESP_ERROR_CHECK(gptimer_enable(my_timer));
    ESP_LOGI("", "Enabled timer");

    ESP_ERROR_CHECK(gptimer_set_alarm_action(my_timer, &alarm_config));
}
