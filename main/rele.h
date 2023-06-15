#ifndef _RELE_H
#define _RELE_H

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


//how to use relay module - define class

class Rele {
public:
    Rele();
    void on();
    void off();
    void toggle();
    void test();
    void print();
    const char* tag = "Rele";
    QueueHandle_t queue;

    static void control_loop(void* args);

private:
    // Definiçoes das Portas Digitais do Arduino
    gpio_num_t m_rele; // Porta digital D08 - reset do A4988
};

void rele_control_loop(void* args);







#endif // _RELE_H