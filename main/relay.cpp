#include "relay.h"

const char* Relay::tag = "Relay";

Relay::Relay(gpio_num_t pino) {
    // Defina o número do pino apropriado para o relé
    m_relay = pino;
    // Configura o pino do relé como saída
    gpio_set_direction(m_relay, GPIO_MODE_OUTPUT);

    gpio_set_level(m_relay, LOW); // Desliga o relé inicialmente
}

void Relay::on() {
    gpio_set_level(m_relay, HIGH); // Liga o relé
    state = true; // Atualiza o estado para ligado
}

void Relay::off() {
    gpio_set_level(m_relay, LOW); // Desliga o relé
    state = false; // Atualiza o estado para desligado
}

void Relay::toggle() {
    gpio_set_level(m_relay, !gpio_get_level(m_relay)); // Inverte o estado do relé
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

    while (true) {
    char command = 0;
        Command c   = {
                .type  = (Type)0,
                .value = 0,
        };


        ESP_LOGI(rel->tag,"relay control | relay.queue %p", rel->queue);
        ESP_LOGI(tag, "relay control 0x%02X", c.type);
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
