#include "wifi_utils.h"

WifiState wifiState = WIFI_UNCONFIGURED;

const char * system_event_reasons1[] = { "UNSPECIFIED", "AUTH_EXPIRE", "AUTH_LEAVE", "ASSOC_EXPIRE", "ASSOC_TOOMANY", "NOT_AUTHED", "NOT_ASSOCED", "ASSOC_LEAVE", "ASSOC_NOT_AUTHED", "DISASSOC_PWRCAP_BAD", "DISASSOC_SUPCHAN_BAD", "IE_INVALID", "MIC_FAILURE", "4WAY_HANDSHAKE_TIMEOUT", "GROUP_KEY_UPDATE_TIMEOUT", "IE_IN_4WAY_DIFFERS", "GROUP_CIPHER_INVALID", "PAIRWISE_CIPHER_INVALID", "AKMP_INVALID", "UNSUPP_RSN_IE_VERSION", "INVALID_RSN_IE_CAP", "802_1X_AUTH_FAILED", "CIPHER_SUITE_REJECTED", "BEACON_TIMEOUT", "NO_AP_FOUND", "AUTH_FAIL", "ASSOC_FAIL", "HANDSHAKE_TIMEOUT" };
#define reason2str(r) ((r>176)?system_event_reasons1[r-176]:system_event_reasons1[r-1])

ServerCondition::ServerCondition(ServerError err){
  error = err;
  numberOfTimeouts = 0;
}

void ServerCondition::resetError(){
  error = NO_ERROR;
  numberOfTimeouts = 0;
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    uint8_t* reason;
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
          LOGD(WIFI_U, "Wifi STA started.");
          wifiState = WIFI_STARTED;
        break;
        case SYSTEM_EVENT_STA_STOP:
          LOGD(WIFI_U, "Wifi STA stopped.");
        break;
        case SYSTEM_EVENT_STA_CONNECTED:
          LOGD(WIFI_U, "Wifi connected.");
        break;
        case SYSTEM_EVENT_STA_GOT_IP:
          LOGI(WIFI_U, "Wifi got ip %s",
            ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
          wifiState = WIFI_CONNECTED;
        break;
        case SYSTEM_EVENT_STA_LOST_IP:
          LOGD(WIFI_U, "Wifi lost ip.");
        break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
          LOGD(WIFI_U, "WiFi disconnected. ");
          reason = &event->event_info.disconnected.reason;
          LOGD(WIFI_U, "Reason: %u - %s\n", *reason, reason2str(*reason));
          wifiState = WIFI_STARTED;
        break;
        default:
          LOGW(WIFI_U, "[WiFi-event] Unhandled event with ID %d\n", event->event_id);
        break;
    }
    return ESP_OK;
}

bool wifi_init() {
  LOGI(WIFI_U, "Init event loop...");
  esp_err_t err = esp_event_loop_init(event_handler, NULL);
  if (err != ESP_OK) {
    LOGE(WIFI_U, "Cannot init event loop. Error: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}

bool wifi_start_sta(String ssid, String password, 
    IPAddress ip, IPAddress gateway, IPAddress subnet)
{
  LOGI(WIFI_U, "Init TCPIP...");
  tcpip_adapter_init();

  tcpip_adapter_ip_info_t info;
  info.ip.addr = static_cast<uint32_t>(ip);
  info.gw.addr = static_cast<uint32_t>(gateway);
  info.netmask.addr = static_cast<uint32_t>(subnet);

  esp_err_t err;
  
  LOGI(WIFI_U, "Init Wifi...");
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  
  err =  esp_wifi_init(&cfg);
  if (err != ESP_OK) {
    LOGE(WIFI_U, "Cannot init wifi. Error: %s", esp_err_to_name(err));
    return false;
  }

 err = esp_wifi_set_mode(WIFI_MODE_STA);
  if (err != ESP_OK) {
    LOGE(WIFI_U, "Cannot set wifi sta mode. Error: %s", esp_err_to_name(err));    
    return false;
  }

  LOGI(WIFI_U, "Configuring Wifi with SSID:%s...\n", ssid.c_str());
  wifi_config_t sta_config = { };
  strcpy((char*)sta_config.sta.ssid, ssid.c_str());
  strcpy((char*)sta_config.sta.password, password.c_str());
  sta_config.sta.bssid_set = false;

  err = esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config);
  if (err != ESP_OK) {
    LOGE(WIFI_U, "Cannot set wifi config. Error: %s", esp_err_to_name(err));
    return false;
  }
  LOGI(WIFI_U, "Starting Wifi...");
  err = esp_wifi_start();
  if (err != ESP_OK) {
    LOGE(WIFI_U, "Cannot start wifi. Error: %s", esp_err_to_name(err));
    return false;
  }

  err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
  if (err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED){
      LOGE(WIFI_U, "DHCP could not be stopped! Error: %s", esp_err_to_name(err));
      return false;
  }

  err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &info);
  if (err != ESP_OK) {
      LOGE(WIFI_U, "STA IP could not be configured! Error:%s", esp_err_to_name(err));
      return false;
  }


  LOGI(WIFI_U, "End of wifi_start_sta.");
  return true;
}

void wifi_stop_sta() {
  LOGI(WIFI_U, "Stopping Wifi...");
  esp_wifi_stop();
  
  LOGI(WIFI_U, "Deinitializing Wifi...");
  esp_wifi_deinit();

  wifiState = WIFI_UNCONFIGURED;
  LOGI(WIFI_U, "End of wifi_stop_sta.");
}

String wiFiStateToString(){
  
  switch(wifiState){
    case WIFI_UNCONFIGURED:
    return String("UNCONFIGURED");

    case WIFI_STARTED:
    return String("WIFI_STARTED");
    
    case WIFI_CONNECTED:
    return String("WIFI_CONNECTED");
  } 
  return String("UNKNOWN WiFi STATE");
}

String serverStateToString(ServerState state){
	switch(state){
    case UNCONFIGURED:
      return String( "UNCONFIGURED");
    case DISCONNECTED:
      return String("DISCONNECTED");
    case CONNECTED:
      return String("CONNECTED");
    case SERVER_LISTENING:
      return String("SERVER_LISTENING");
    case CLIENT_CONNECTED:
      return String("CLIENT_CONNECTED");
    case DATA_AVAILABLE:
      return String("DATA_AVAILABLE");
    default:
      return String("UNKNOWN_STATE");
  } 
  return String("UNKNOWN_STATE");
}

ServerState getNextServerStateUp(ServerState state) {
	switch(state){
    case UNCONFIGURED:
      return DISCONNECTED;
    case DISCONNECTED:
      return CONNECTED;
    case CONNECTED:
      return SERVER_LISTENING;
    case SERVER_LISTENING:
      return CLIENT_CONNECTED;
    case CLIENT_CONNECTED:
      return DATA_AVAILABLE;
    case DATA_AVAILABLE:
      return DATA_AVAILABLE;
    default:
      return state;
  } 
}

ServerState getNextServerStateDown(ServerState state) {
	switch(state){
    case DATA_AVAILABLE:
      return CLIENT_CONNECTED;
    case CLIENT_CONNECTED:
      return SERVER_LISTENING;
    case SERVER_LISTENING:
      return CONNECTED;
    case CONNECTED:
      return DISCONNECTED;
    case DISCONNECTED:
      return UNCONFIGURED; 
    case UNCONFIGURED:
      return UNCONFIGURED;
    default:
      return state;
  } 
}


String Transition::toString(){
  
  if (isEmptyTransition()) {
    return String("No action");
  }
  else {
    char buffer[40];
    snprintf(buffer, 40, "%s --> %s", 
		  serverStateToString(from).c_str(), serverStateToString(to).c_str());
		return String(buffer);
  }
}
