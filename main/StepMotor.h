#pragma once

// https://www.fernandok.com/2019/06/motor-inteligente.html

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "data_command_event.h"
#include "pins.h"

#define HIGH 1
#define LOW  0

typedef enum {
    STOPPED,
    RUNNING,
    TEST,
} MotorState;

#define UP   true
#define DOWN false

class StepMotor {
public:
    StepMotor();
    static void control_loop(void* args);
    static bool IRAM_ATTR incomplete_step(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data);
    void start();
    void stop();
    void set_speed(int speed);
    void set_direction(int dir);

    static const char* tag;
    bool step_state      = 0;
    bool dir_state       = UP;
    int double_the_steps = 0;

    const gpio_num_t pin_direction;
    const gpio_num_t pin_step;
    const gpio_num_t pin_enable;
    const gpio_num_t pin_led;
    // const gpio_num_t pin_alarm;
    // const gpio_num_t pin_pend;

    QueueHandle_t queue;
    gptimer_handle_t gptimer;
    gptimer_alarm_config_t alarm_config;

private:
    MotorState state;
    MotorState prev_state;

    void gptimer_init();

    const int STEPS_PER_REVOLUTION = 400; // muda de acordo com o chaveamento
    // const int STEPS_PER_REVOLUTION = 800; // muda de acordo com o chaveamento
};
