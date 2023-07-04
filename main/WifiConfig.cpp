#include "WifiConfig.h"

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
        // Faça o processamento dos dados recebidos aqui
        // imprimir os dados recebidos
        ESP_LOGI(TAG, "Dados recebidos: %.*s", ret, buf);
        // salvar os dados recebidos na memória flash
        nvs_handle_t my_handle;
        esp_err_t err;
        // abrir a partição nvs
        err = nvs_open("storage", NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erro ao abrir a partição nvs");
        }
        // gravar os dados recebidos na partição nvs
        err = nvs_set_str(my_handle, "wifi_config", buf);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erro ao gravar os dados na partição nvs");
        }
        // fechar a partição nvs
        nvs_close(my_handle);
    
    }

    // Responda com uma mensagem de sucesso e a página atualizada
    const char *resp_str = "<html><body><h1>Configuração salva!</h1><p>As informações foram enviadas com sucesso.</p>";
    // Acrescente aqui o conteúdo HTML da página que deseja carregar após o envio

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}







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

int32_t readNVS()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    nvs_handle partitionHandler;

    esp_err_t openResponse = nvs_open("WifiStorage",NVS_READONLY,&partitionHandler);

    if(openResponse == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG,"Namespace: WifiStorage not found");
        return -1;
    }

    int32_t value;

    esp_err_t response = nvs_get_i32(partitionHandler,"testNum",&value);

    switch (response)
    {
    case ESP_OK:
        printf("Value was read %ld",value);
        ESP_LOGI(TAG,"Value was read %ld",value);
        break;
    
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE(TAG,"SSID NOT FOUND.");
        value = -1;
        break;
    default:
        ESP_LOGE(TAG,"Error to access nvs(%s).",esp_err_to_name(response));
        value = -1;
        break;
    }

    nvs_close(partitionHandler);
    return value;
}

void writeNVS(int32_t value)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    nvs_handle partitionHandler;

    esp_err_t openResponse = nvs_open("WifiStorage",NVS_READWRITE,&partitionHandler);

    if(openResponse == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG,"Namespace: WifiStorage not found");

    }

    esp_err_t response = nvs_set_i32(partitionHandler,"testNum",value + 1);

    if(response != ESP_OK)
    {
        ESP_LOGE(TAG,"Error to write nvs(%s).",esp_err_to_name(response));

    }

    nvs_close(partitionHandler);
}