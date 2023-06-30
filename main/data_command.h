#pragma once

#include "cJSON.h"
#include "esp_log.h"

#define ON  1
#define OFF 0

// Comandos mais alto nível que chegam do frontend
typedef enum {
    T_NONE       = 0x00,
    T_VELOCITY   = 0xC1, // Não vai ser usado
    T_MAX_HEIGHT = 0xC2, // Altura em centímetros
    T_MIN_HEIGHT = 0xC3, // Altura em centímetros
    T_ON_OFF     = 0xC4, // 0 desligado, 1 ligado
} Type;

typedef struct {
    Type type;
    int value;
} Command;

// Atuador ou sensor
typedef enum {
    D_PRESSURE   = 0xD1,
    D_STEP_MOTOR = 0xD2,
    D_RELAY      = 0xD3,
} Device;

typedef enum {
    SMDT_NONE     = 0x00,
    SMDT_STEPS    = 0xA1,
    SMDT_POSITION = 0xA2,
    SMDT_TURNS    = 0xA3,
    SMDT_ON_OFF   = 0xA4,
} StepMotorDataType;

typedef struct {
    StepMotorDataType type;
    int value;
} StepMotorData;

typedef enum {
    RID_SOLENOID   = 0xF1,
    RID_COMPRESSOR = 0xF2,
} RelayID;

typedef struct {
    int on_off;
    RelayID id;
} RelayData;

typedef struct {
    int value;
} PressureData;

typedef struct {
    Device device;
    union {
        StepMotorData step_motor;
        RelayData relay;
        PressureData pressure;
    };
} AllData;

void to_json(cJSON* root, AllData* data);