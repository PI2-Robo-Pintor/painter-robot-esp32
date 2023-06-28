#include "StepMotor.h"

const char* StepMotor::tag = "StepMotor";

StepMotor::StepMotor(void) {
    state = STOPPED;

    pin_direction = PIN_SM_DIRECTION;
    pin_step      = PIN_SM_STEP;
    pin_led       = PIN_SM_LED;

    // Apenas teste de LED interna
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    gpio_set_direction(pin_direction, GPIO_MODE_OUTPUT);
    gpio_set_direction(pin_step, GPIO_MODE_OUTPUT);
    // gpio_set_direction(pin_enable, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_direction, dir_state);

    meioPeriodo = 1000;
    PPS         = 0;
    sentido     = true;
    PPR         = 200;
    voltas      = 3;
    gptimer     = NULL;

    gptimer_init();
}

void StepMotor::gptimer_init() {
    ESP_LOGI(tag, "Create timer handle");
    gptimer = NULL;
    gptimer_config_t timer_config;
    timer_config.clk_src       = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction     = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz, 1 tick=1us

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = incomplete_step,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, this));

    ESP_LOGI(tag, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    alarm_config.reload_count               = 0;
    alarm_config.alarm_count                = 400;
    alarm_config.flags.auto_reload_on_alarm = true;

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
}

void StepMotor::control_loop(void* args) {
    StepMotor* motor = (StepMotor*)args;

    int counter = 0;

    while (true) {
        counter++;
        char command = 0;
        Command c    = {
               .type  = (Type)0,
               .value = 0,
        };

        // BaseType_t result = xQueueReceive(motor->queue, &command, portMAX_DELAY);
        BaseType_t result = xQueueReceive(motor->queue, &c, portMAX_DELAY);
        if (result != pdPASS) {
            // ESP_LOGE(tag, "Erro na fila?");
            // continue;
        }

        ESP_LOGI(tag, "control: command 0x%02X", c.type);

        if (command == 'x') {
            motor->state = RUNNING;
            gpio_set_level(GPIO_NUM_2, HIGH);
        } else if (command == 'z') {
            motor->state = STOPPED;
            gpio_set_level(GPIO_NUM_2, LOW);
        } else if (command == 'y') {
            motor->dir_state = !motor->dir_state;
            gpio_set_level(motor->pin_direction, motor->dir_state);
            ESP_LOGI(tag, "invertendo sentido de deslocamento");
        } else if ('a' <= command && command <= 'f') {
            int step_delay                                 = 80 + (command - 'a') * 20;
            motor->alarm_config.alarm_count                = step_delay;
            motor->alarm_config.reload_count               = 0;
            motor->alarm_config.flags.auto_reload_on_alarm = true;
            gptimer_stop(motor->gptimer);
            gptimer_set_alarm_action(motor->gptimer, &motor->alarm_config);
            gptimer_start(motor->gptimer);
        } else if (command == 0) {
            ESP_LOGW(tag, "Nenhum comando");
        } else {
            ESP_LOGE(tag, "Comando não reconhecido: [%c]", command);
        }

        switch (motor->state) {
        case RUNNING:
            if (motor->prev_state != RUNNING)
                gptimer_start(motor->gptimer);
            break;
        case STOPPED:
            // FIXME: tenho que rever como vai ser a atualização do estado
            gptimer_stop(motor->gptimer);
            break;
        default:
            break;
        }
    }
}

bool StepMotor::incomplete_step(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data) {
    BaseType_t high_task_awoken = pdFALSE;
    StepMotor* motor            = (StepMotor*)user_data;
    motor->counter++;
    motor->step_state = !motor->step_state;
    gpio_set_level(motor->pin_step, motor->step_state);
    return (high_task_awoken == pdTRUE);
}
