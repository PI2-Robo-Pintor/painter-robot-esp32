#pragma once

#include "driver/gpio.h"

#include "esp_event.h"
#include "esp_log.h"


#define WIFI_SSID           "AndroidAP1FDB"
#define WIFI_PASSWORD       "vmds9571"
#define WIFI_CONN_MAX_RETRY 6
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static void eventHandler(void* arguments, 
                  esp_event_base_t eventBase,
                  int32_t eventId,
                  void* eventData);
    

void WifiStart();
