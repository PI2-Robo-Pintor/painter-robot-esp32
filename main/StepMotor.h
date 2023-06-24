#pragma once

// https://www.fernandok.com/2019/06/motor-inteligente.html

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "pins.h"

#define HIGH 1
#define LOW  0

typedef enum {
    STOPPED,
    RUNNING,
    TEST,
} MotorState;

class StepMotor {
public:
    StepMotor();
    void reset();
    void disable();
    void enable();
    void clockwise();
    void counterClockwise();
    void step();
    void frequency();
    void fullStep();
    void halfStep();
    void microStep4();
    void microStep8();
    void microStep16();
    void testMotor();
    void printRpm();
    static const char* tag;
    QueueHandle_t queue;

    static void control_loop(void* args);

private:
    MotorState state;

    // Definiçoes das Portas Digitais do Arduino
    gpio_num_t pin_direction;
    gpio_num_t pin_step;
    gpio_num_t pin_enable;
    gpio_num_t pin_alarm;
    gpio_num_t pin_pend;
    gpio_num_t pin_led;
    const int STEPS_PER_REVOLUTION = 400; // muda de acordo com o chaveamento
    // const int STEPS_PER_REVOLUTION = 800; // muda de acordo com o chaveamento
    // 10us roda bem. Devemos descobrir pq não roda bem com 2us
    const int TIME_HIGH_MS = 1;

    int meioPeriodo = 1000; // MeioPeriodo do pulso STEP em microsegundos F= 1/T = 1/2000 uS = 500 Hz
    float PPS       = 0;    // Pulsos por segundo
    bool sentido    = true; // Variavel de sentido
    long PPR        = 200;  // Número de passos por volta
    long pulsos;            // Pulsos para o driver do motor
    int voltas = 3;         // voltas do motor
    float RPM;              // Rotacoes por minuto
};
