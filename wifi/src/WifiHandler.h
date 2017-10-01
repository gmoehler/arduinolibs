#ifndef WIFI_HANDLER
#define WIFI_HANDLER

#include <WiFi.h>

enum WifiState {  DISCONNECTED,
                  CONNECTED,
                  CONNECTED_WITH_IP,
                  SERVER_LISTENING,
                  CLIENT_CONNECTED,
                  CLIENT_DATA_AVAILABLE
};

class WifiHandler
{
public:
  WifiHandler(IPAddress ip, IPAddress gateway, IPAddress subnet, 
    uint16_t serverPort, char* ssid, char* wifiPassword);

  void setTargetState(WifiState targetState);
  WifiState getState();

  void loop();

private:
  WifiState _targetState;
  WifiState _currentState;

  IPAddress _ip; 
  IPAddress _gateway; 
  IPAddress _subnet; 
  uint16_t _serverPort;
  char* _ssid; 
  char* _wifiPassword;


};

#endif