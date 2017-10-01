#ifndef WIFI_HANDLER
#define WIFI_HANDLER

#include <WiFi.h>

enum WifiState {  DISCONNECTED,
                  CONNECTING,
                  CONNECTED,
                  CONNECTED_WITH_IP,
                  SERVER_LISTENING,
                  CLIENT_CONNECTED,
                  DATA_AVAILABLE
};

class WifiHandler
{
public:
  static void init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
    uint16_t serverPort, char* ssid, char* wifiPassword);

  static void setTargetState(WifiState targetState);
  static WifiState getState();

  static void loop();

private:
  static WifiState _targetState;
  static WifiState _currentState;
  static WifiState _nextState;

  static WiFiServer _server;

  static IPAddress _ip; 
  static IPAddress _gateway; 
  static IPAddress _subnet; 
  static uint16_t _serverPort;
  static char* _ssid; 
  static char* _wifiPassword;

  static void onWiFiEvent(WiFiEvent_t event);

};

#endif