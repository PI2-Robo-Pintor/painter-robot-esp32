#pragma once

// https://www.fernandok.com/2019/06/motor-inteligente.html

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "data_command_event.h"
#include "pins.h"

extern QueueHandle_t mainQueue;
extern QueueHandle_t sensorsQueue;

#define HIGH 1
#define LOW  0

typedef enum {
    STOPPED,
    RUNNING,
    TEST,
} MotorState;

typedef enum {
    D_UP   = +1,
    D_DOWN = -1,
} MotorDirection;

#define UP   true
#define DOWN false

class StepMotor {
public:
    StepMotor();
    static void control_loop(void* args);
    static bool IRAM_ATTR incomplete_step(gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_data);

    void start();
    void stop();
    void set_delay(int speed);
    void set_direction(MotorDirection dir);
    void set_target_position(int pos);
    // Retorna true se já está lá
    bool go_to(int target_steps);

    static const char* tag;
    bool step_state          = 0;
    MotorDirection dir_state = D_UP;
    int double_the_steps     = 0;
    int target_position      = 0;

    int target_delay;
    int initial_delay;

    static const int STEPS_PER_REVOLUTION = 400; // muda de acordo com o chaveamento
    const gpio_num_t pin_direction;
    const gpio_num_t pin_step;
    // const gpio_num_t pin_enable;
    const gpio_num_t pin_led;

    // const gpio_num_t pin_alarm;
    // const gpio_num_t pin_pend;

    QueueHandle_t queue;
    gptimer_handle_t step_timer;
    gptimer_handle_t acc_timer;
    gptimer_alarm_config_t alarm_config;

private:
    MotorState state;
    MotorState prev_state;

    // void gptimer_init(gptimer_handle_t timer, gptimer_alarm_cb_t callback);
    void gptimer_init();
};
