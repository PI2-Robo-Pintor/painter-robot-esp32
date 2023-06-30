#include "data_command.h"

void to_json(cJSON* root, AllData* data) {
    cJSON_AddNumberToObject(root, "device", data->device);

    switch (data->device) {
    case D_STEP_MOTOR:
        cJSON_AddNumberToObject(root, "type", data->step_motor.type);
        cJSON_AddNumberToObject(root, "value", data->step_motor.value);
        break;
    case D_RELAY:
        cJSON_AddNumberToObject(root, "on_off", data->relay.on_off);
        cJSON_AddNumberToObject(root, "id", data->relay.id);
        break;

    case D_PRESSURE:
        cJSON_AddNumberToObject(root, "value", data->pressure.value);
        break;
    default:
        ESP_LOGW("to_json", "Device not recognized 0x%X", data->device);
        break;
    }
};
