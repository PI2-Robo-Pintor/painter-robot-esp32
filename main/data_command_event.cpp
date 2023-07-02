#include "data_command_event.h"

EventCommand event_command_reset() {
    EventCommand ec;
    ec.type          = T_NONE;
    ec.event.type    = E_NONE;
    ec.command.type  = T_NONE;
    ec.command.value = 0;
    return ec;
}

void to_json(cJSON* root, AllData* data) {
    cJSON_AddNumberToObject(root, "device", data->device);

    switch (data->device) {
    case D_STEP_MOTOR:
        cJSON_AddNumberToObject(root, "type", data->step_motor.type);
        cJSON_AddNumberToObject(root, "value", data->step_motor.value);
        break;
    case D_RELAY:
        cJSON_AddNumberToObject(root, "value", data->relay.value);
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
