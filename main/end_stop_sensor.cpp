#include "end_stop_sensor.h"

#include "driver/gptimer.h"

const char* tag = "End stop sensor";

extern QueueHandle_t mainQueue;

static TickType_t last_interrupt;

static const TickType_t THRESHOLD = 20;

gptimer_handle_t read_end_stop_timer = NULL;

// Outra altenativa: a primeira interrupção desabilita a interrupção logo
// no começo do handler.

void setup_end_stop_sensor();

static bool IRAM_ATTR end_stop_reader_handler(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data) {
    BaseType_t high_task_awoken = pdFALSE;

    EventCommand event = event_command_reset();
    event.type         = T_EVENT;
    int level          = gpio_get_level(PIN_END_STOP);
    if (level == HIGH)
        event.event.type = E_JUST_RELEASED_END_STOP_SENSOR;
    else
        event.event.type = E_JUST_PRESSED_END_STOP_SENSOR;

    xQueueSendFromISR(mainQueue, &event, NULL);

    return (high_task_awoken == pdTRUE);
};

void init_read_end_stop_timer() {
    gptimer_alarm_config_t alarm_config;
    alarm_config.alarm_count                = 50'000;
    alarm_config.reload_count               = 0;
    alarm_config.flags.auto_reload_on_alarm = false;

    gptimer_config_t timer_config;
    timer_config.clk_src       = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction     = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz, 1 tick=1us

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &read_end_stop_timer));
    gptimer_event_callbacks_t cbs = {
        .on_alarm = end_stop_reader_handler,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(read_end_stop_timer, &cbs, NULL));

    ESP_ERROR_CHECK(gptimer_enable(read_end_stop_timer));
    // ESP_LOGI(tag, "Enabled timer");

    ESP_ERROR_CHECK(gptimer_set_alarm_action(read_end_stop_timer, &alarm_config));
}

static void IRAM_ATTR handle_end_stop(void* args) {
    TickType_t now = xTaskGetTickCountFromISR();

    if (now - last_interrupt > THRESHOLD) {
        gpio_intr_disable(PIN_END_STOP);
        gptimer_start(read_end_stop_timer);
    }

    last_interrupt = now;
}

void setup_end_stop_sensor() {
    // ESP_LOGI(tag, "Create timer handle");

    gpio_set_direction(PIN_END_STOP, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_END_STOP);
    gpio_set_intr_type(PIN_END_STOP, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_END_STOP, handle_end_stop, NULL);

    init_read_end_stop_timer();

    last_interrupt = xTaskGetTickCount();
};

void reenable_end_stop_sensor() {
    gptimer_stop(read_end_stop_timer);
    gptimer_disable(read_end_stop_timer);
    gptimer_del_timer(read_end_stop_timer);

    init_read_end_stop_timer();

    gpio_intr_enable(PIN_END_STOP);
}