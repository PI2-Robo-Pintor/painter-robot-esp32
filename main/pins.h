#pragma once

// #define PIN_SM_DIRECTION GPIO_NUM_4
// #define PIN_SM_STEP      GPIO_NUM_5

// FIXME: esses pinos são temporários, enquanto os anteriores não estão
// funcionando
#define PIN_SM_DIRECTION GPIO_NUM_23
#define PIN_SM_STEP      GPIO_NUM_19

#define PIN_SM_LED    GPIO_NUM_3
#define PIN_SM_ENABLE GPIO_NUM_22
#define PIN_SM_PEND   GPIO_NUM_7
#define PIN_SM_ALARM  GPIO_NUM_8

// Sensor de fimde curso
#define PIN_END_STOP GPIO_NUM_21

// Relé
#define PIN_RELAY_1 GPIO_NUM_19
#define PIN_RELAY_2 GPIO_NUM_22

// Sensor Pressão
#define PIN_PSENSOR_DATA GPIO_NUM_33
#define PIN_PSENSOR_SCK GPIO_NUM_32

// RGB LED
#define PIN_LED_G GPIO_NUM_25
#define PIN_LED_R GPIO_NUM_26
#define PIN_LED_Y GPIO_NUM_27
