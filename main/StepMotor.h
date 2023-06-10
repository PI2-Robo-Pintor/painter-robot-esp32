#ifndef STEPMOTOR_H_INCLUDED
#define STEPMOTOR_H_INCLUDED

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tag.h"

#define RST 8
#define SLP 9
#define ENA 7
#define MS1 4
#define MS2 5
#define MS3 6
#define DIR 3
#define STP 2

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

#endif // STEPMOTOR_H_INCLUDED
