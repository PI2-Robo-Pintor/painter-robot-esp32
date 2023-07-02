#include "WifiConfig.h"

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define TAG "Wifi"

static int s_connectTries = 0;
static EventGroupHandle_t s_wifiEventGroup;

static void eventHandlerSta(void* arguments, 
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

            if (s_connectTries < WIFI_CONN_MAX_RETRY)
            {
                esp_wifi_connect();
                
                ++s_connectTries;

                ESP_LOGI(TAG,"Retry to connect to the Ap.");
            }
            else
            {
                xEventGroupSetBits(s_wifiEventGroup,WIFI_FAIL_BIT);
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

            s_connectTries = 0;

            xEventGroupSetBits(s_wifiEventGroup,WIFI_CONNECTED_BIT);
        }
        
    }
    

}

void WifiStartSta()
{
    s_wifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,&eventHandlerSta, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,&eventHandlerSta, NULL));

    wifi_config_t config = {
        .sta = {
            .ssid = WIFI_SSID_STA,
            .password = WIFI_PASSWORD_STA
        },
        
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifiEventGroup,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG,"Connected to ap: SSID: %s PASSWORD: %s ",WIFI_SSID_STA,WIFI_PASSWORD_STA);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG,"Failed to ap: SSID: %s PASSWORD: %s ",WIFI_SSID_STA,WIFI_PASSWORD_STA);
    }
    else
    {
        ESP_LOGI(TAG,"UNEXPECTED EVENT");
    }
    
    vEventGroupDelete(s_wifiEventGroup);
}


static void eventHandlerAp(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void WifiStartSoftAp()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventHandlerAp,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID_AP,
            .password = WIFI_PASSWORD_AP,
            .ssid_len = strlen(WIFI_SSID_AP),
            .channel = WIFI_CHANNEL,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = WIFI_CONN_MAX_RETRY,
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    if (strlen(WIFI_PASSWORD_AP) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WifiStartSoftAp finished. SSID:%s password:%s channel:%d",
             WIFI_SSID_AP, WIFI_PASSWORD_AP, WIFI_CHANNEL);
}

void readNVS()
{
    ESP_ERROR_CHECK(nvs_flash_init);

    nvs_handle partitionHandler;

    esp_err_t openResponse = nvs_open("WifiStorage",NVS_READONLY,&partitionHandler);

    if(openResponse == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG,"Namespace: WifiStorage not found");
        return;
    }

    int32_t value;

    esp_err_t response = nvs_get_i32(partitionHandler,"testNum",&value);

    switch (response)
    {
    case ESP_OK:
        s_ssid = ssid;
        break;
    
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG,"SSID NOT FOUND.");
        break;
    default:
        ESP_LOGE(TAG,"Error to access nvs(%s).",esp_err_to_name(response));
        break;
    }

    nvs_close(partitionHandler);
}