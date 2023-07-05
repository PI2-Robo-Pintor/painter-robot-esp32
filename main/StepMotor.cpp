#include "StepMotor.h"
#include <stdlib.h>

const char* StepMotor::tag = "StepMotor";

StepMotor::StepMotor()
    : pin_direction(PIN_SM_DIRECTION),
      pin_step(PIN_SM_STEP),
      pin_enable(PIN_SM_ENABLE),
      pin_led(PIN_SM_LED) {

    state = STOPPED;

    // Apenas teste de LED interna
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    gpio_set_direction(pin_direction, GPIO_MODE_OUTPUT);
    gpio_set_direction(pin_step, GPIO_MODE_OUTPUT);
    gpio_set_direction(pin_enable, GPIO_MODE_OUTPUT);

    gpio_set_level(pin_direction, dir_state);

    // gptimer_init(step_timer, incomplete_step);
    gptimer_init();

    stop();
}

void StepMotor::gptimer_init() {
    // void StepMotor::gptimer_init(gptimer_handle_t timer, gptimer_alarm_cb_t callback) {
    ESP_LOGI(tag, "Create timer handle");
    step_timer = NULL;
    gptimer_config_t timer_config;
    timer_config.clk_src       = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction     = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz, 1 tick=1us

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &step_timer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = incomplete_step,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(step_timer, &cbs, this));

    ESP_ERROR_CHECK(gptimer_enable(step_timer));
    ESP_LOGI(tag, "Enabled step timer");

    initial_delay                           = 900;
    target_delay                            = 100;
    alarm_config.alarm_count                = initial_delay;
    alarm_config.reload_count               = 0;
    alarm_config.flags.auto_reload_on_alarm = true;

    ESP_ERROR_CHECK(gptimer_set_alarm_action(step_timer, &alarm_config));
}

void StepMotor::start() {
    this->state = RUNNING;

    set_delay(initial_delay);

    gpio_set_level(GPIO_NUM_2, HIGH);
    gpio_set_level(pin_enable, LOW);
    // FIXME: E se o timer já tiver começado?
    gptimer_start(this->step_timer);
}

void StepMotor::stop() {
    this->state = STOPPED;
    gpio_set_level(pin_enable, HIGH);
    gpio_set_level(GPIO_NUM_2, LOW);
    ESP_LOGI(tag, "passos       %d", this->double_the_steps);
    gptimer_stop(this->step_timer);
}

void StepMotor::set_delay(int delay) {
    // Por enquanto esse speed é um delay em microsegundos
    // int step_delay                                = 80 + (speed - 'a') * 20;
    this->alarm_config.alarm_count                = delay;
    this->alarm_config.reload_count               = 0;
    this->alarm_config.flags.auto_reload_on_alarm = true;
    gptimer_stop(this->step_timer);
    gptimer_set_alarm_action(this->step_timer, &this->alarm_config);
    // Não necessariamente eu preciso mudar a velocidade agr
    gptimer_start(this->step_timer);
}

void StepMotor::set_direction(MotorDirection dir) {
    this->dir_state = dir;
    if (dir == D_UP)
        gpio_set_level(this->pin_direction, HIGH);
    else
        gpio_set_level(this->pin_direction, LOW);
    ESP_LOGI(tag, "invertendo sentido de deslocamento %d", this->dir_state);
}

void StepMotor::control_loop(void* args) {
    StepMotor* motor = (StepMotor*)args;

    int counter = 0;

    while (true) {
        counter++;
        Command c = {
            .type  = T_NONE,
            .value = 0,
        };

        BaseType_t result = xQueueReceive(motor->queue, &c, 1000 / portTICK_PERIOD_MS);
        if (result != pdPASS) {
            // ESP_LOGE(tag, "Erro na fila?");
            // continue;
        }

        AllData data = {
            .device     = D_STEP_MOTOR,
            .step_motor = {
                .type  = SMDT_POSITION,
                .value = motor->double_the_steps,
            }};

        xQueueSend(sensorsQueue, &data, 1);
    }
}

void StepMotor::set_height_stop(int cm) {
    height_stop = cm;
}

bool StepMotor::incomplete_step(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data) {
    BaseType_t high_task_awoken = pdFALSE;
    StepMotor* motor            = (StepMotor*)user_data;

    const int upper_paintable_limit = 350'000;
    if (motor->double_the_steps + 1 == upper_paintable_limit) {
        EventCommand event = event_command_reset();
        event.type         = T_EVENT;
        event.event.type   = E_REACHED_UPPER_LIMIT;
        // FIXME: deveria ter prioridade máxima
        xQueueSendFromISR(mainQueue, &event, NULL);
    }
    if (motor->double_the_steps - 1 == 0) {
    }

    if (motor->dir_state == UP) {
        motor->double_the_steps++;
    } else
        motor->double_the_steps--;

    if (abs(motor->double_the_steps) % 5 == 0) {
        int delta_delay = 1;

        int delay_diff = motor->target_delay - motor->alarm_config.alarm_count;
        if (delay_diff > 0) {
        } else if (delay_diff < 0) {
            delta_delay = -delta_delay;
        }

        if (delay_diff != 0) {
            int new_delay = motor->alarm_config.alarm_count + delta_delay;
            motor->set_delay(new_delay);
        }
    }

    // SS = a / 360 * f * 60
    //  f = SS * 360 / (60 * a)

    motor->step_state = !motor->step_state;
    gpio_set_level(motor->pin_step, motor->step_state);
    return (high_task_awoken == pdTRUE);
}
