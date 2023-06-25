#include "relay.h"


Relay::Relay(void) {
    // Defina o número do pino apropriado para o relé
    m_relay = PIN_RELAY;
    // Configura o pino do relé como saída
    gpio_set_direction(PIN_RELAY, GPIO_MODE_OUTPUT);

    gpio_set_level(PIN_RELAY, LOW); // Desliga o relé inicialmente
}

void Relay::on() {
    gpio_set_level(PIN_RELAY, HIGH); // Liga o relé
    state = true; // Atualiza o estado para ligado
}

void Relay::off() {
    gpio_set_level(PIN_RELAY, LOW); // Desliga o relé
    state = false; // Atualiza o estado para desligado
}

void Relay::toggle() {
    gpio_set_level(PIN_RELAY, !gpio_get_level(PIN_RELAY)); // Inverte o estado do relé
    state = !state; // Atualiza o estado para o novo valor
}

bool Relay::getState() {
    //imprimir o estado do relé
    return state; // Retorna o estado atual do relé
}

void Relay::test() {
    on();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
    off();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
}

void Relay::print() {
    ESP_LOGI(tag, "Relé: %s", getState() ? "Ligado" : "Desligado");
}


void Relay::blink() {
    // Loop para piscar o relé
    while (true) {
        // Liga o relé
        on();
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms

        // Desliga o relé
        off();
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms
    }
}

void Relay::blinkLoop(void* args) {
    Relay* rel = (Relay*)args;

    while (true) {
        rel->blink(); // Chama a função de piscar o relé
    }
}


void Relay::control_loop_relay(void* args) {
    Relay* rel = (Relay*)args;

    char command = 0;
    while (true) {
        ESP_LOGI(rel->tag,"relay control | relay.queue %p", rel->queue);
        if(xQueueReceive(rel->queue, &command, portMAX_DELAY) == pdPASS) {
            if (command == 'r') {   
                rel->on();
            } else {
                rel->off();
            }
        }
        //ESPLOGI RELAY GETSTATE  
        rel->print();

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Aguarda 1 segundo
    }
} 

