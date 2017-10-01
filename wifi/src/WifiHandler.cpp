#include "WifiHandler.h"

// initialize static members
WifiState WifiHandler::_targetState = DISCONNECTED;
WifiState WifiHandler::_currentState = DISCONNECTED;
WiFiServer WifiHandler::_server;

IPAddress WifiHandler::_ip; 
IPAddress WifiHandler::_gateway; 
IPAddress WifiHandler::_subnet; 
uint16_t WifiHandler::_serverPort;
char* WifiHandler::_ssid; 
char* WifiHandler::_wifiPassword;

void WifiHandler::onWiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_WIFI_READY:
      printf("Wifi ready.\n");
      _currentState = DISCONNECTED;
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      printf("Wifi scan done.\n");
      break;
    case SYSTEM_EVENT_STA_START:
      printf("Wifi started...\n");
      _currentState = DISCONNECTED;
      break;
    case SYSTEM_EVENT_STA_STOP:
      printf("Wifi disconnected.\n");
      _currentState = DISCONNECTED;
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      printf("Wifi connected.\n");
      _currentState = CONNECTED;
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      printf("Authmode of Access Point has changed.\n");
      _currentState = DISCONNECTED;
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      printf("IP address: %s\n", WiFi.localIP().toString().c_str());
      _currentState = CONNECTED_WITH_IP;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("WiFi lost connection.\n");
      _currentState = DISCONNECTED;
      break;
    default:
      printf("[WiFi-event] Unhandled event: %d\n", event);
      break;
  }
}


void WifiHandler::init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
  uint16_t serverPort, char* ssid, char* wifiPassword) {
  _ip = ip;
  _gateway = gateway;
  _subnet = subnet;
  _serverPort = serverPort;
  _ssid = ssid;
  _wifiPassword = wifiPassword;
  _targetState = DISCONNECTED; 
  _currentState = DISCONNECTED;

  _server = WiFiServer(serverPort);
  WiFi.onEvent(onWiFiEvent);
  }

void WifiHandler::setTargetState(WifiState targetState){
  _targetState = targetState;
}

WifiState WifiHandler::getState() {
  return _currentState;
}


void WifiHandler::loop(){
  if (_currentState == _targetState){
    return;
  } 

  switch(_currentState){
    
    case DISCONNECTED:
      WiFi.mode(WIFI_STA);  // "station" mode
      WiFi.disconnect();    // to be on the safe side
      WiFi.config(_ip, _gateway, _subnet);  // set specific ip...
      WiFi.begin(_ssid, _wifiPassword);     // connect to router
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      } 
      break;

    case CONNECTED:

      break;

    case CONNECTED_WITH_IP:

      break;

    case SERVER_LISTENING:

      break;

    case CLIENT_CONNECTED:

      break;

    case DATA_AVAILABLE:

      break;

     default:
      break;
  } 

}
