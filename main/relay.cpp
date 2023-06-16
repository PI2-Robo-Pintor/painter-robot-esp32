#include "relay.h"


Relay::Relay() {
    // Defina o número do pino apropriado para o relé
    m_relay = PIN_RELAY;
    // Configura o pino do relé como saída
    gpio_set_direction(m_relay, GPIO_MODE_OUTPUT);

    gpio_set_level(m_relay, 0); // Desliga o relé inicialmente
}

void Relay::on() {
    gpio_set_level(m_relay, 1); // Liga o relé
}

void Relay::off() {
    gpio_set_level(m_relay, 0); // Desliga o relé
}

void Relay::toggle() {
    gpio_set_level(m_relay, !gpio_get_level(m_relay)); // Inverte o estado do relé

}
void Relay::test() {
    on();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
    off();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
}

void Relay::print() {
    ESP_LOGI(tag, "Relé: %s", gpio_get_level(m_relay) ? "Ligado" : "Desligado");
}

void Relay::control_loop(void* args) {
    Relay* relay = static_cast<Relay*>(args);
    char command = 0;

    while (true) {
        ESP_LOGI(relay->tag,"relay control | relay.queue: %p", relay->queue);
        if(xQueueReceive(relay->queue, &command, (TickType_t)1) == pdPASS) {
            if (command == 'a') {
                relay->on(); //liga o rele
            } else {
                relay->off();//desliga o rele
            }
        relay->print(); // Exibe o estado do relé
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 0,5 segundos
    }
}
