#pragma once

#include "cJSON.h"
#include "esp_log.h"

#define ON  1
#define OFF 0

typedef enum {
    E_NONE                          = 0x00,
    E_JUST_PRESSED_END_STOP_SENSOR  = 0xE1,
    E_JUST_RELEASED_END_STOP_SENSOR = 0xE2,
    E_REACHED_UPPER_LIMIT           = 0xE3,
} EventType;

typedef struct {
    EventType type;
} Event;

// Comandos mais alto nível que chegam do frontend
typedef enum {
    T_NONE            = 0x00,
    T_VELOCITY        = 0xC1, // Não vai ser usado
    T_MAX_HEIGHT      = 0xC2, // Altura em centímetros
    T_MIN_HEIGHT      = 0xC3, // Altura em centímetros
    T_ON_OFF          = 0xC4, // 0 desligado, 1 ligado
    T_INVERT          = 0xC5, // inverte direção do deslocamento. NÃO deve ser usado em produção
    T_TEST_COMPONENTS = 0xC6, // testa motor, compressor,
} Type;

typedef struct {
    Type type;
    int value;
} Command;

typedef enum {
    T_EVENT   = 1,
    T_COMMAND = 2,
} T;

typedef struct {
    int type;
    union {
        Event event;
        Command command;
    };
} EventCommand;

EventCommand event_command_reset();

// Atuador ou sensor
typedef enum {
    D_PRESSURE   = 0xD1,
    D_STEP_MOTOR = 0xD2,
    D_RELAY      = 0xD3,
    D_ROBOT      = 0xD4,
} Device;

typedef enum {
    SMDT_NONE     = 0x00,
    SMDT_POSITION = 0xA1,
    SMDT_ON_OFF   = 0xA2,
    SMDT_STEPS    = 0xA3,
    SMDT_TURNS    = 0xA4,
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
    int value;
    RelayID id;
} RelayData;

typedef struct {
    int value;
} PressureData;

typedef enum {
    RDT_NONE       = 0x00,
    RDT_COMPRESSOR = 0xB1,
    RDT_MOTOR      = 0xB2,
} RobotDataType;

typedef struct {
    RobotDataType type;
    int value; // 0 = OK, 1 ou qq outra coisa = ERRO
} RobotData;

// FIXME: Esse nome é temporário até encontrar um melhor
typedef struct {
    Device device;
    union {
        RelayData relay;
        PressureData pressure;
        StepMotorData step_motor;
    };
} AllData;

void to_json(cJSON* root, AllData* data);
