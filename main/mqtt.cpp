#include "mqtt.h"

const char* Mqtt::TAG              = "Mqtt";
const char* Mqtt::TOPIC_SENSORS    = "pi2/sensors";
const char* Mqtt::TOPIC_STEP_MOTOR = "pi2/step-motor";
const char* Mqtt::TOPIC_SOLENOID   = "pi2/solenoid";

void Mqtt::log_error_if_nonzero(const char* message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void Mqtt::start() {
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri       = "ws://test.mosquitto.org";
    mqtt_cfg.broker.address.port      = 8080;

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, handle_event, this);
    esp_mqtt_client_start(client);
}

void Mqtt::handle_event(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event   = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;

    Mqtt* mqtt = (Mqtt*)handler_args;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client, TOPIC_STEP_MOTOR, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, TOPIC_SOLENOID, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, TOPIC_SENSORS, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        handle_event_data(mqtt, event);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void Mqtt::handle_event_data(Mqtt* mqtt, esp_mqtt_event_handle_t event) {
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    // Por algum motivo event->topic não aponta pra um buffer
    // com \0 no final.
    char topic[128];
    memcpy(topic, event->topic, event->topic_len);
    topic[event->topic_len] = 0;

    printf("TOPIC=%.*s .. %s\n", event->topic_len, event->topic, topic);
    printf(" DATA=%.*s\n", event->data_len, event->data);

    char command = event->data[0];

    if (strcmp(topic, TOPIC_STEP_MOTOR) == 0) {
        if (xQueueSend(mqtt->stepMotorQueue, &command, 0) == pdPASS) {
            ESP_LOGI(TAG, "MQTT mensagem enviada p/ StepMotor Queue");
        } else
            ESP_LOGW(TAG, "FALHA MQTT mensagem NÃO enviada p/ StepMotor Queue");
    } else if (strcmp(topic, TOPIC_SOLENOID) == 0) {
        if (xQueueSend(mqtt->solenoidQueue, &command, 0) == pdPASS) {
            ESP_LOGI(TAG, "MQTT mensagem enviada p/ Solenoid Queue");
        } else
            ESP_LOGW(TAG, "FALHA MQTT mensagem NÃO enviada p/ Solenoid Queue");
    } else {
        ESP_LOGW(TAG, "Tópico não reconehcido %s", topic);
    }
}