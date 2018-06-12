
#include "wifi_utils.h"

ServerCondition::ServerCondition(ServerError err){
  error = err;
  numberOfTimeouts = 0;
}

void ServerCondition::resetError(){
  error = NO_ERROR;
  numberOfTimeouts = 0;
}

String wiFiStateToString(){
  
  wl_status_t wifiState = WiFi.status();

  switch(wifiState){
    case WL_NO_SHIELD:
    return String("WL_NO_SHIELD");

    case WL_IDLE_STATUS:
    return String("WL_IDLE_STATUS");
    
    case WL_NO_SSID_AVAIL:
    return String("WL_NO_SSID_AVAIL");

    case WL_SCAN_COMPLETED:
    return String("WL_SCAN_COMPLETED");

    case WL_CONNECTED:
    return String("WL_CONNECTED");

    case WL_CONNECT_FAILED:
    return String("WL_CONNECT_FAILED");

    case WL_CONNECTION_LOST:
    return String("WL_CONNECTION_LOST");

    case WL_DISCONNECTED:
    return String("WL_DISCONNECTED");
  } 
  return String("UNKNOWN WiFi STATE");
}


String serverStateToString(ServerState state){
	switch(state){
    case ERR_SSID_NOT_AVAIL:
      return String( "ERR_SSID_NOT_AVAIL");
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

const char * system_event_reasons1[] = { "UNSPECIFIED", "AUTH_EXPIRE", "AUTH_LEAVE", "ASSOC_EXPIRE", "ASSOC_TOOMANY", "NOT_AUTHED", "NOT_ASSOCED", "ASSOC_LEAVE", "ASSOC_NOT_AUTHED", "DISASSOC_PWRCAP_BAD", "DISASSOC_SUPCHAN_BAD", "IE_INVALID", "MIC_FAILURE", "4WAY_HANDSHAKE_TIMEOUT", "GROUP_KEY_UPDATE_TIMEOUT", "IE_IN_4WAY_DIFFERS", "GROUP_CIPHER_INVALID", "PAIRWISE_CIPHER_INVALID", "AKMP_INVALID", "UNSUPP_RSN_IE_VERSION", "INVALID_RSN_IE_CAP", "802_1X_AUTH_FAILED", "CIPHER_SUITE_REJECTED", "BEACON_TIMEOUT", "NO_AP_FOUND", "AUTH_FAIL", "ASSOC_FAIL", "HANDSHAKE_TIMEOUT" };
#define reason2str(r) ((r>176)?system_event_reasons1[r-176]:system_event_reasons1[r-1])

void onWiFiEvent(WiFiEvent_t event, system_event_info_t event_info)
{
  uint8_t reason;
  printf(">> Wifi Event: ");
  switch (event) {
    case SYSTEM_EVENT_WIFI_READY:
      printf("Wifi ready.\n");
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      printf("Wifi scan done.\n");
      break;
    case SYSTEM_EVENT_STA_START:
      printf("Wifi started...\n");
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
      printf("IP address: %s\n", WiFi.localIP().toString().c_str());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("WiFi lost connection.\n");
      reason = event_info.disconnected.reason;
      printf("Reason: %u - %s\n", reason, reason2str(reason));
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
    case SYSTEM_EVENT_AP_START:
      printf("AP start\n");
      break;
    case SYSTEM_EVENT_AP_STOP:
      printf("AP stop\n");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      printf("station connected to AP\n");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      printf("station disconnected from AP\n");
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      printf("Received probe request packet in soft-AP interface\n");
      break;
    case SYSTEM_EVENT_GOT_IP6:
      printf("ESP32 station or ap or ethernet interface v6IP addr is preferred\n");
      break;
    case SYSTEM_EVENT_ETH_START:
      printf("ESP32 ethernet start\n");
      break;
    case SYSTEM_EVENT_ETH_STOP:
      printf("ESP32 ethernet stop\n");
      break;
   case SYSTEM_EVENT_ETH_CONNECTED:
      printf("ESP32 ethernet phy link up\n");
      break;
   case SYSTEM_EVENT_ETH_DISCONNECTED:
      printf("ESP32 ethernet phy link down\n");
      break;
   case SYSTEM_EVENT_ETH_GOT_IP:
      printf("ESP32 ethernet got IP from connected AP\n");
      break;
    default:
      printf("[WiFi-event] Unhandled event: %d\n", event);
      break;
  }
}

ServerState getNextServerStateUp(ServerState state) {
	switch(state){
    case ERR_SSID_NOT_AVAIL:
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
      return DISCONNECTED; // will not go to ERR_SSID_NOT_AVAIL
    case ERR_SSID_NOT_AVAIL:
      return ERR_SSID_NOT_AVAIL;
    default:
      return state;
  } 
}


String Transition::toString(){
  
  if (withAction()) {
    char buffer[40];
    snprintf(buffer, 40, "%s --> %s", 
		  serverStateToString(from).c_str(), serverStateToString(to).c_str());
		return String(buffer);
  }
  else {
    return String("No action");
  }
}
