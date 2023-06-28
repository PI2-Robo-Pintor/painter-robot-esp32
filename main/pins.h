#pragma once

#include "driver/gpio.h"



// Motor de passo.  Esses pinos são temporários
#define PIN_SM_DIRECTION GPIO_NUM_4
#define PIN_SM_STEP      GPIO_NUM_5

#define PIN_SM_LED    GPIO_NUM_3
#define PIN_SM_ENABLE GPIO_NUM_22
#define PIN_SM_PEND   GPIO_NUM_7
#define PIN_SM_ALARM  GPIO_NUM_8

// Sensor de fimde curso
#define PIN_END_STOP GPIO_NUM_21

// Relé
#define PIN_RELAY_2 GPIO_NUM_20
#define PIN_RELAY_1 GPIO_NUM_23
