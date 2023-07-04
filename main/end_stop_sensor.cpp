#include "end_stop_sensor.h"

extern QueueHandle_t mainQueue;

static int64_t last_negative_edge;

void setup_end_stop_sensor() {
    gpio_set_direction(PIN_END_STOP, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_END_STOP);
    gpio_set_intr_type(PIN_END_STOP, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_END_STOP, handle_end_stop, NULL);

    last_negative_edge = esp_timer_get_time();
};

void handle_end_stop(void* args) {

    int64_t now = esp_timer_get_time();

    if (now - last_negative_edge > 10'000) {
        EventCommand event;
        event.type          = T_EVENT;
        event.event.type    = E_REACHED_LOWER_END_STOP_SENSOR;
        event.command.value = gpio_get_level(PIN_END_STOP);
        xQueueSendFromISR(mainQueue, &event, NULL);
    }

    last_negative_edge = now;
}
