/* Simple WiFi Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   from https://github.com/espressif/esp-idf/blob/master/examples/wifi/simple_wifi/main/simple_wifi.c
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "IPAddress.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define SSID "MY_SSID7"
#define PASSWORD "my_password"

const String mySsid     = "MY_SSID7";
const String myPassword = "my_password";


#define EXAMPLE_MAX_STA_CONN       CONFIG_MAX_STA_CONN

IPAddress myIP(192,168,1,127);
IPAddress myGateway(192, 168, 1, 1);
IPAddress mySubnet(255, 255, 255, 0);
uint16_t port = 1110;

int level = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

const char * system_event_reasons[] = { "UNSPECIFIED", "AUTH_EXPIRE", "AUTH_LEAVE", "ASSOC_EXPIRE", "ASSOC_TOOMANY", "NOT_AUTHED", "NOT_ASSOCED", "ASSOC_LEAVE", "ASSOC_NOT_AUTHED", "DISASSOC_PWRCAP_BAD", "DISASSOC_SUPCHAN_BAD", "IE_INVALID", "MIC_FAILURE", "4WAY_HANDSHAKE_TIMEOUT", "GROUP_KEY_UPDATE_TIMEOUT", "IE_IN_4WAY_DIFFERS", "GROUP_CIPHER_INVALID", "PAIRWISE_CIPHER_INVALID", "AKMP_INVALID", "UNSUPP_RSN_IE_VERSION", "INVALID_RSN_IE_CAP", "802_1X_AUTH_FAILED", "CIPHER_SUITE_REJECTED", "BEACON_TIMEOUT", "NO_AP_FOUND", "AUTH_FAIL", "ASSOC_FAIL", "HANDSHAKE_TIMEOUT" };
#define reason2str(r) ((r>176)?system_event_reasons[r-176]:system_event_reasons[r-1])

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    uint8_t* reason;
    switch(event->event_id) {
        case SYSTEM_EVENT_WIFI_READY:
        printf("Wifi ready.\n");
        break;
        case SYSTEM_EVENT_SCAN_DONE:
        printf("Wifi scan done.\n");
        break;
        case SYSTEM_EVENT_STA_START:
        printf("sta started.\n");
        esp_wifi_connect();
        break;
        case SYSTEM_EVENT_STA_STOP:
        printf("Wifi disconnected.\n");
        break;
        case SYSTEM_EVENT_STA_CONNECTED:
        printf("Wifi connected.\n");
        break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        printf("Authmode of Access Point has changed.\n");
        break;
        case SYSTEM_EVENT_STA_GOT_IP:
        printf( "got ip:%s\n",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
        printf("WiFi disconnected ");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        reason = &event->event_info.disconnected.reason;
        printf("Reason: %u - %s\n", reason, reason2str(*reason));
        break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        printf("WiFi connected by WPS.\n");
        break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        printf("WPS connection failed.\n");
        break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        printf("WPS connection timeout.\n");
        break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
        printf("WPS with pin mode: %s\n"/*, PIN2STR(event.event_info.sta_er_pin.pin_code)*/);
        break;
        default:
        printf("[WiFi-event] Unhandled event: %d\n", event);
        break;
    }
    return ESP_OK;
}


bool wifi_init_sta(String ssid, String password, 
    IPAddress ip, IPAddress gateway, IPAddress subnet)
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    if (ESP_OK != esp_event_loop_init(event_handler, NULL) ){
        return false;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (ESP_OK != esp_wifi_init(&cfg)){
        return false;
    }

    wifi_config_t sta_config = { };
    strcpy((char*)sta_config.sta.ssid, ssid.c_str());
    strcpy((char*)sta_config.sta.password, password.c_str());
    sta_config.sta.bssid_set = false;

    if (ESP_OK != esp_wifi_set_mode(WIFI_MODE_STA) ){
        return false;
    }
    if (ESP_OK != esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config) ){
        return false;
    }
    if (ESP_OK != esp_wifi_start() ){
        return false;
    }

    tcpip_adapter_ip_info_t info;

    if(myIP != (uint32_t)0x00000000){
        info.ip.addr = static_cast<uint32_t>(myIP);
        info.gw.addr = static_cast<uint32_t>(gateway);
        info.netmask.addr = static_cast<uint32_t>(subnet);
    } else {
        info.ip.addr = 0;
        info.gw.addr = 0;
        info.netmask.addr = 0;
    }

    esp_err_t err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    if(err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED){
        printf("DHCP could not be stopped! Error: %d", err);
        return false;
    }

    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &info);
    if(err != ERR_OK){
        printf("STA IP could not be configured! Error: %d", err);
        return false;
    }

    printf( "wifi_init_sta finished.\n");
    printf( "connect to ap SSID:%s password:%s\n",
             SSID, PASSWORD);
}

void setup()
{
    //Initialize NVS
    printf( "init nvs.\n");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    printf( "ESP_WIFI_MODE_STA\n");
    wifi_init_sta(mySsid, myPassword, myIP, myGateway, mySubnet);

    vTaskDelay(6000 / portTICK_PERIOD_MS);
    printf("disconnecting...\n");
    esp_wifi_disconnect();

    vTaskDelay(6000 / portTICK_PERIOD_MS);
    printf("stopping...\n");
    esp_wifi_stop();

}


void loop()
{
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // gpio_set_level(GPIO_NUM_4, level);
    // level = !level;
}