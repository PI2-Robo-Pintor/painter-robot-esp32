#pragma once

#include "pins.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define HIGH 1
#define LOW  0
//how to use relay module - define class

class Relay {
public:
    Relay(gpio_num_t pino);
    void on();
    void off();
    void toggle();
    void test();
    void print();
    const char* tag = "Relay";
    QueueHandle_t queue;

    bool state = false;
    bool getState();

    static void control_loop_relay(void* args);

    void blink();
    static void blinkLoop(void* args);

private:
    // Definiçoes das Portas Digitais do Arduino
    gpio_num_t m_relay;
};

void relay_control_loop(void* args);
