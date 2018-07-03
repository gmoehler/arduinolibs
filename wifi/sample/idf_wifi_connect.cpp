// sample from https://github.com/espressif/esp-idf-template/blob/master/main/main.c

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <string.h>

int level = 0;
#define SSID "MY_SSID"
#define PASSWORD "my_password"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}


void setup()
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

    wifi_config_t sta_config = { };
    strcpy((char*)sta_config.sta.ssid, SSID);
    strcpy((char*)sta_config.sta.password, PASSWORD);
    sta_config.sta.bssid_set = false;

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
}

void loop()
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    gpio_set_level(GPIO_NUM_4, level);
    level = !level;
}
