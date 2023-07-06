#include "SystemStateLed.h"

const char* SystemStateLed::tag = "SystemStateLed";

// --- Configurações Iniciais ---
SystemStateLed::SystemStateLed(void) {
  gpio_set_direction(PIN_LED_G, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_LED_Y, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_LED_R, GPIO_MODE_OUTPUT);
}

void SystemStateLed::green() {
  gpio_set_level(PIN_LED_G, HIGH);
  gpio_set_level(PIN_LED_Y, LOW);
  gpio_set_level(PIN_LED_R, LOW);
}

void SystemStateLed::yellow() {
  gpio_set_level(PIN_LED_G, LOW);
  gpio_set_level(PIN_LED_Y, HIGH);
  gpio_set_level(PIN_LED_R, LOW);
}

void SystemStateLed::red() {
  gpio_set_level(PIN_LED_G, LOW);
  gpio_set_level(PIN_LED_Y, LOW);
  gpio_set_level(PIN_LED_R, HIGH);
}
