#ifndef WIFI_HANDLER
#define WIFI_HANDLER

/**
 * Robust wifi handler
 * 
 * Can handle the following wifi situations:
 * - client connects
 * - client sends data
 * - client disconnects
 * - client is interrupted during transmission
 * - wifi stops and is restarted when no client is connected
 * - wifi stops and is restarted when client is connected (not tested yet)
 * - wifi stops during client transmission (still issues with that)
 * 
 **/


#include <WiFi.h>

enum WifiState {  DISCONNECTED,
                  CONNECTED,
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
  static char readData();

  static void loop();

private:
  static WifiState _currentState; // current state
  static WifiState _targetState;  // ultimate target
  static WifiState _nextState;    // next state to reach

  static WiFiServer _server;
  static WiFiClient _client;

  static IPAddress _ip; 
  static IPAddress _gateway; 
  static IPAddress _subnet; 
  static uint16_t _serverPort;
  static char* _ssid; 
  static char* _wifiPassword;

  static WifiState _determineNextState(bool upward);
  static void _invokeAction(bool upward);
  static bool _checkState(WifiState state);
  static void _printState(WifiState state);

  static bool _errorSituation;

};

#endif