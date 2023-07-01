#include "WifiConfig.h"


#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#define WIFI_SSID="myssid"
#define WIFI_PASSWORD="mypassword"
#define WIFI_CONN_MAX_RETRY=6

#define TAG "Wifi"

void WifiConfig::WifiStart()
{

}

void EventHandlerCallback(WifiConfig *classInstance) {
   classInstance->eventHandler();
}

WifiConfig::WifiConfig(/* args */)
{
    m_wifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,&EventHandlerCallback, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP,&EventHandlerCallback, this));

    wifi_config_t config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA,&config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(m_wifiEventGroup,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG,"Connected to ap: SSID: %s PASSWORD: %s ",WIFI_SSID,WIFI_PASSWORD);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG,"Failed to ap: SSID: %s PASSWORD: %s ",WIFI_SSID,WIFI_PASSWORD);
    }
    else
    {
        ESP_LOGI(TAG,"UNEXPECTED EVENT");
    }
    
    vEventGroupDelete(m_wifiEventGroup);
}

void WifiConfig::eventHandler(void* arguments, 
                 esp_event_base_t eventBase,
                 int32_t eventId,
                 void* eventData)
{
    if (eventBase == WIFI_EVENT )
    {
        if (eventId == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
        }
        else if (eventId == WIFI_EVENT_STA_DISCONNECTED)
        {

            if (m_connectTries < WIFI_CONN_MAX_RETRY)
            {
                esp_wifi_connect();
                
                ++m_connectTries;

                ESP_LOGI(TAG,"Retry to connect to the Ap.");
            }
            else
            {
                xEventGroupSetBits(m_wifiEventGroup,WIFI_FAIL_BIT);
            }
        
            ESP_LOGI(TAG,"Connect to the Ap fail.");
            
        }
        
        
    }
    else
    {
        if ((eventBase == IP_EVENT) && (eventId == IP_EVENT_STA_GOT_IP))
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;

            ESP_LOGI(TAG,"Ip address received: " IPSTR,IP2STR(&event->ip_info.ip));

            m_connectTries = 0;

            xEventGroupSetBits(m_wifiEventGroup,WIFI_CONNECTED_BIT);
        }
        
    }
    

}