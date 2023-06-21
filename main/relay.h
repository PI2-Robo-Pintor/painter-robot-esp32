#pragma once

#include "pins.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


//how to use relay module - define class

class Relay {
public:
    Relay();
    void on();
    void off();
    void toggle();
    void test();
    void print();
    const char* tag = "Relay";
    QueueHandle_t queue;

    static void control_loop(void* args);

private:
    // Definiçoes das Portas Digitais do Arduino
    gpio_num_t m_relay; // Porta digital D08 - reset do A4988
};

void relay_control_loop(void* args);
