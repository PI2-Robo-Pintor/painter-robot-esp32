#pragma once

#include "driver/gpio.h"

#include "esp_event.h"
#include "esp_log.h"


#define WIFI_CONN_MAX_RETRY 6
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

#define WIFI_SSID_STA   "AndroidAP1FDB"
#define WIFI_PASSWORD_STA   "vmds9571"

#define WIFI_SSID_AP    "robopintor"
#define WIFI_PASSWORD_AP    "12345678"
#define WIFI_CHANNEL    0



static void eventHandlerAp(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);

static void eventHandlerSta(void* arguments, 
                  esp_event_base_t eventBase,
                  int32_t eventId,
                  void* eventData);
    

void WifiStartSta();
void WifiStartSoftAp();

char* readNVS();
void writeNVSStr(char* valueStr);