#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "data_command_event.h"
#include "pins.h"

#define HIGH 1
#define LOW  0
// how to use relay module - define class

class Relay {
public:
    Relay(gpio_num_t pino);
    void on();
    void off();
    void toggle();
    void test();
    void print();
    QueueHandle_t queue;

    static const char* tag;
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
