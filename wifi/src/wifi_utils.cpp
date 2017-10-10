
#include "wifi_utils.h"

void printWiFiState( bool withPrintln){
  
  wl_status_t wifiState = WiFi.status();

  switch(wifiState){
    case WL_NO_SHIELD:
    Serial.print("WL_NO_SHIELD");
    break;
    case WL_IDLE_STATUS:
    Serial.print("WL_IDLE_STATUS");
    break;    
    case WL_NO_SSID_AVAIL:
    Serial.print("WL_NO_SSID_AVAIL");
    break;
    case WL_SCAN_COMPLETED:
    Serial.print("WL_SCAN_COMPLETED");
    break;
    case WL_CONNECTED:
    Serial.print("WL_CONNECTED");
    break;
    case WL_CONNECT_FAILED:
    Serial.print("WL_CONNECT_FAILED");
    break;
    case WL_CONNECTION_LOST:
    Serial.print("WL_CONNECTION_LOST");
    break;
    case WL_DISCONNECTED:
    Serial.print("WL_DISCONNECTED");
    break;
    default:
    Serial.print("UNKNOWN WiFi STATE");
    break;
  } 
  if (withPrintln){
    Serial.println();
  }
}

void printServerState(ServerState state, bool withPrintln){
  switch(state){
    case DISCONNECTED:
    Serial.print("DISCONNECTED");
    break;
    case ERR_SSID_NOT_AVAIL:
    Serial.print("ERR_SSID_NOT_AVAIL");
    break;
    case CONNECTED:
    Serial.print("CONNECTED");
    break;
    case SERVER_LISTENING:
    Serial.print("SERVER_LISTENING");
    break;
    case CLIENT_CONNECTED:
    Serial.print("CLIENT_CONNECTED");
    break;
    case DATA_AVAILABLE:
    Serial.print("DATA_AVAILABLE");
    break;
    default:
    Serial.print("UNKNOWN_STATE");
    break;
  } 
  if (withPrintln){
    Serial.println();
  }
}

void printTransition(Transition trans, bool withPrintln){
  if (trans.withAction()) {
    printServerState(trans.from);
    Serial.print(" --> ");
    printServerState(trans.to);
  }
  else {
    Serial.print("No action");
  }
  if (withPrintln){
    Serial.println();
  }
}

