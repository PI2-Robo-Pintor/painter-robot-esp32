#include "PressureSensor.h"

const char* PressureSensor::tag = "PressureSensor";

// --- Configurações Iniciais ---
PressureSensor::PressureSensor(void) {
  gpio_set_direction(PIN_PSENSOR_DATA, GPIO_MODE_INPUT);
  gpio_set_direction(PIN_PSENSOR_SCK, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_2, HIGH);
}

void PressureSensor::measure_loop(void* args) {
  PressureSensor* sensor = (PressureSensor*)args;
  int pressure;

  while (true) {
    pressure = (int)sensor->read_count()*40/167772;
    AllData data = {
      .device = D_PRESSURE,
      .pressure  = {
        .value = pressure,
      },
    };
    ESP_LOGI(tag, "Pressão: %d", pressure);

    if (xQueueSend(sensor->queue,  &data, 0) == pdPASS) {
      ESP_LOGI(tag, "MQTT mensagem enviada p/ main control loop");
      // imprimir comando e valor recebido
    } else
      ESP_LOGW(tag, "FALHA MQTT mensagem NÃO p/ main control loop");
    ESP_LOGI(tag, "Pressão2: %d", pressure);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
  }
}

unsigned long PressureSensor::read_count() {
  unsigned long count = 0;
  unsigned char i;

  gpio_set_level(PIN_PSENSOR_SCK, LOW);

  while(gpio_get_level(PIN_PSENSOR_DATA));

  for(i = 0; i < 24; i++) {
    gpio_set_level(PIN_PSENSOR_SCK, HIGH);
    count = count << 1;
    gpio_set_level(PIN_PSENSOR_SCK, LOW);
    if(gpio_get_level(PIN_PSENSOR_DATA)) count++;
  }

  gpio_set_level(PIN_PSENSOR_SCK, HIGH);
  count = count^0x800000;
  gpio_set_level(PIN_PSENSOR_SCK, LOW);

  return(count);
}
