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