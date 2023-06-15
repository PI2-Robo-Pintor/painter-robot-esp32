#include reles.h

Rele:: Rele(void) {
    m_rele = GPIO_NUM_8;

    // gpio set_on_level(m_rele, LOW);
    gpio_set_on_level(m_rele, HIGH);
    gpio_set_off_level(m_rele, LOW);
}
