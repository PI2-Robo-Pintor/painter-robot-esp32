#pragma once

#include "driver/gpio.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
class WifiConfig
{
private:
    void eventHandler(void* arguments, 
                 esp_event_base_t eventBase,
                 int32_t eventId,
                 void* eventData);
    
    int m_connectTries = 0;
    EventGroupHandle_t m_wifiEventGroup;

public:
    WifiConfig(/* args */);
    ~WifiConfig() = default;

    void WifiStart();
};


WifiConfig::~WifiConfig()
{
}
