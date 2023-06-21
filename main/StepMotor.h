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
    const char* tag = "StepMotor";
    QueueHandle_t queue;

    static void control_loop(void* args);

private:
    // Definiçoes das Portas Digitais do Arduino
    gpio_num_t m_RST; // Porta digital D08 - reset do A4988
    gpio_num_t m_SLP; // Porta digital D09 - dormir (sleep) A4988
    gpio_num_t m_ENA; // Porta digital D07 - ativa (enable) A4988
    gpio_num_t m_MS1; // Porta digital D04 - MS1 do A4988
    gpio_num_t m_MS2; // Porta digital D05 - MS2 do A4988
    gpio_num_t m_MS3; // Porta digital D06 - MS3 do A4988
    gpio_num_t m_DIR; // Porta digital D03 - direção (direction) do A4988
    gpio_num_t m_STP; // Porta digital D02 - passo(step) do A4988

    int meioPeriodo = 1000; // MeioPeriodo do pulso STEP em microsegundos F= 1/T = 1/2000 uS = 500 Hz
    float PPS       = 0;    // Pulsos por segundo
    bool sentido    = true; // Variavel de sentido
    long PPR        = 200;  // Número de passos por volta
    long pulsos;            // Pulsos para o driver do motor
    int voltas = 3;         // voltas do motor
    float RPM;              // Rotacoes por minuto
};
