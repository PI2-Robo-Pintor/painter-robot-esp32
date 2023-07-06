#include "WifiConfig.h"

#include <cJSON.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_http_server.h"

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


// função para tratar as requisições http
esp_err_t root_get_handler(httpd_req_t *req)
{
   // ler arquivo configwifi.html
    extern const unsigned char conectwifi_html_start[] asm("_binary_conectwifi_html_start");
   
    // tamanho do arquivo configwifi.html
    extern const unsigned char conectwifi_html_end[] asm("_binary_conectwifi_html_end");

    // httpd_resp_send(req, (const char *) configwifi_html_start, configwifi_html_end - configwifi_html_start);
    httpd_resp_send(req, (const char *) conectwifi_html_start, HTTPD_RESP_USE_STRLEN);


    return ESP_OK;
}


esp_err_t config_post_handler(httpd_req_t *req) 
{
    char buf[1000];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, sizeof(buf))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }

        remaining -= ret;
        // Processar os dados recebidos aqui
        cJSON *json = cJSON_Parse(buf);
        if (json == NULL) {
            ESP_LOGE(TAG, "Falha ao analisar JSON");
            return ESP_FAIL;
        }

        cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
        cJSON *password = cJSON_GetObjectItem(json, "password");

        if (ssid == NULL || password == NULL) {
            ESP_LOGE(TAG, "Campos 'ssid' ou 'password' ausentes no JSON");
            cJSON_Delete(json);
            return ESP_FAIL;
        }

        const char *ssidValue = ssid->valuestring;
        const char *passwordValue = password->valuestring;
        //imprimir os valores recebidos
        ESP_LOGI(TAG, "ssid: %s", ssidValue);
        ESP_LOGI(TAG, "password: %s", passwordValue);


        // Gravar no NVS
       writeNVS(ssidValue, passwordValue);
       // LER DO NVS E IMPRIMIR LOGI DO SSID E PASSWORD
   
        char* flashSSID; 
        char* flashPassword; 

        readNVS(&flashSSID, &flashPassword);

        if (flashSSID != NULL) {
            ESP_LOGI(TAG, "NVS - SSID: %s", flashSSID);
            free(flashSSID);
        } else {
            ESP_LOGE(TAG, "Failed to read SSID from NVS.");
        }

        if (flashPassword != NULL) {
            ESP_LOGI(TAG, "NVS - Password: %s", flashPassword);
            free(flashPassword);
        } else {
            ESP_LOGE(TAG, "Failed to read password from NVS.");
        }

                cJSON_Delete(json);
            }

    return ESP_OK;
}


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

char* convert_ssid_to_char(const char* ssid)
{
    char* converted_ssid = (char*)malloc(32 * sizeof(char));

    strncpy(converted_ssid, ssid, 32);

    return converted_ssid;
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

    char* flashSSID; 
    char* flashPassword; 

    readNVS(&flashSSID, &flashPassword);

    if (flashSSID != NULL) {
         ESP_LOGI(TAG, "NVS - SSID: %s", flashSSID);
        free(flashSSID);
    } else {
         ESP_LOGE(TAG, "Failed to read SSID from NVS.");
    }

    if (flashPassword != NULL) {
        ESP_LOGI(TAG, "NVS - Password: %s", flashPassword);
        free(flashPassword);
    } else {
         ESP_LOGE(TAG, "Failed to read password from NVS.");
    }

     char setSSID[32];

     for (size_t i = 0; i < strlen(flashSSID); i++)
     {
        setSSID[i] = flashSSID[i];
     }
     
     
    
     char setPassword[64];

     for (size_t i = 0; i < strlen(flashPassword); i++)
     {
        setPassword[i] = flashPassword[i];
     }


    wifi_config_t config;

    int i = 0;

     for ( i = 0; i < 32; i++)
     {
        config.sta.ssid[i] = 0;
        
     }

    for (i = 0; i < 64; i++)
     {
        config.sta.password[i] = 0;
        
     }
     
    for ( i = 0; i < strlen(flashSSID); i++)
     {
        config.sta.ssid[i] = flashSSID[i];
        
        if(config.sta.ssid[i] == '.')
        {
            config.sta.ssid[i] = 0;
            break;
        }
     }
     
     
     for ( i = 0; i < strlen(flashPassword); i++)
     {
        config.sta.password[i] = flashPassword[i];
        
        if(config.sta.password[i] == '.')
        {
            config.sta.password[i] = 0;
            break;
        }
     }

     
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
        ESP_LOGI(TAG,"Connected to ap: SSID: %s PASSWORD: %s ",config.sta.ssid,config.sta.password);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG,"Failed to CONNECT to application: SSID: %s PASSWORD: %s ",setSSID,setPassword);
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

    httpd_config_t config_httpd = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;


    ESP_ERROR_CHECK(httpd_start(&server, &config_httpd));

    httpd_uri_t root_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_get_handler,
        .user_ctx  = NULL
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_uri));

    //REGISTRA O HANDLER PARA A ROTA /CONFIG
    httpd_uri_t config_uri = {
        .uri       = "/config",
        .method    = HTTP_POST,
        .handler   = config_post_handler,
        .user_ctx  = NULL
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &config_uri));
}

void readNVS(char** ssidRead, char** senhaRead) {
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        *ssidRead = NULL;
        *senhaRead = NULL;
        return;
    }

    nvs_handle partitionHandler;
    err = nvs_open("WifiStorage", NVS_READONLY, &partitionHandler);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace. Error: %s", esp_err_to_name(err));
        *ssidRead = NULL;
        *senhaRead = NULL;
        return;
    }

    size_t len = 0;
    err = nvs_get_str(partitionHandler, "ssid", NULL, &len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SSID length from NVS. Error: %s", esp_err_to_name(err));
        nvs_close(partitionHandler);
        *ssidRead = NULL;
        *senhaRead = NULL;
        return;
    }

    

    char* ssidValue = (char*)malloc(len + 1); // Alocar memória para o SSID lido
    err = nvs_get_str(partitionHandler, "ssid", ssidValue, &len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read SSID from NVS. Error: %s", esp_err_to_name(err));
        nvs_close(partitionHandler);
        free(ssidValue);
        *ssidRead = NULL;
        *senhaRead = NULL;
        return;
    }

    len = 0;
    err = nvs_get_str(partitionHandler, "senha", NULL, &len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get senha length from NVS. Error: %s", esp_err_to_name(err));
        nvs_close(partitionHandler);
        free(ssidValue);
        *ssidRead = NULL;
        *senhaRead = NULL;
        return;
    }

            char* senhaValue = (char*)malloc(len + 1); // Alocar memória para a senha lida
    err = nvs_get_str(partitionHandler, "senha", senhaValue, &len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read senha from NVS. Error: %s", esp_err_to_name(err));
        nvs_close(partitionHandler);
        free(ssidValue);
        free(senhaValue);
        *ssidRead = NULL;
        *senhaRead = NULL;
        return;
    }

    ESP_LOGI(TAG, "SSID - LIDA: %s", ssidValue);
    ESP_LOGI(TAG, "SENHA - LIDA: %s", senhaValue);
    nvs_close(partitionHandler);

    *ssidRead = ssidValue;
    *senhaRead = senhaValue;
}



void writeNVS(const char* ssid, const char* senha) {
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return;
    }

    nvs_handle partitionHandler;
    err = nvs_open("WifiStorage", NVS_READWRITE, &partitionHandler);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace. Error: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(partitionHandler, "ssid", ssid);
    ESP_LOGI(TAG, "SSID - GRAVADA: %s", ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write SSID to NVS. Error: %s", esp_err_to_name(err));
        nvs_close(partitionHandler);
        return;
    }

    err = nvs_set_str(partitionHandler, "senha", senha);
    ESP_LOGI(TAG, "SENHA - GRAVADA: %s", senha);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write senha to NVS. Error: %s", esp_err_to_name(err));
        nvs_close(partitionHandler);
        return;
    }

    err = nvs_commit(partitionHandler);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit changes to NVS. Error: %s", esp_err_to_name(err));
    }

    nvs_close(partitionHandler);
}
