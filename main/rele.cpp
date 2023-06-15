#include "rele.h"


Rele::Rele() {
    // Defina o número do pino apropriado para o relé
    m_rele = GPIO_NUM_8;
    // Configura o pino do relé como saída
    gpio_pad_select_gpio(m_rele);
    gpio_set_direction(m_rele, GPIO_MODE_OUTPUT);
    gpio_set_level(m_rele, 0); // Desliga o relé inicialmente
}

void Rele::on() {
    gpio_set_level(m_rele, 1); // Liga o relé
}

void Rele::off() {
    gpio_set_level(m_rele, 0); // Desliga o relé
}

void Rele::toggle() {
    int level = gpio_get_level(m_rele);
    gpio_set_level(m_rele, !level); // Inverte o estado do relé
}

void Rele::test() {
    on();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
    off();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
}

void Rele::print() {
    ESP_LOGI(tag, "Relé: %s", gpio_get_level(m_rele) ? "Ligado" : "Desligado");
}

void Rele::control_loop(void* args) {
    Rele* rele = static_cast<Rele*>(args);
    while (true) {
        rele->print(); // Exibe o estado do relé
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 0,5 segundos
    }
}
