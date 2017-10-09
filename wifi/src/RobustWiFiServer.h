#ifndef WIFI_HANDLER
#define WIFI_HANDLER

/**
 * Robust server
 * 
 * Can handle the following wifi situations:
 * 1. client connects
 * 2. client sends data
 * 3. client disconnects when idle
 * 4. client is interrupted during transmission
 * 5. wifi stops and is restarted when no client is connected
 * 6. wifi stops and is restarted when client is connected 
 * 7. wifi stops during client transmission 
 * 
 **/

#include <utility>
#include <WiFi.h>
#include "wifi_utils.h"

class RobustWiFiServer
{
public:
  RobustWiFiServer();
  void init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
    uint16_t serverPort, char* ssid, char* wifiPassword); // to be called in setup()

  void setTargetState(ServerState targetState);
  ServerState getState();

  char readData();  // read data (in state DATA_AVAILABLE)
  size_t writeData(uint8_t data);                     // write (in state CLIENT_CONNECTED)
  size_t writeData(const uint8_t *buf, size_t size);  // write (in state CLIENT_CONNECTED)
 
  void loop();      // to be called in loop()

private:
  Transition _currentTransition; // current state transition
  ServerState _currentState; // current state
  ServerState _targetState;  // ultimate target

  IPAddress _ip; 
  IPAddress _gateway; 
  IPAddress _subnet; 
  uint16_t _serverPort;
  char* _ssid; 
  char* _wifiPassword;

  WiFiServer _server;
  WiFiClient _client;
  
  Transition _determineNextTransition();
  Transition _determineDisconnectTransition();
  
  void _invokeAction(Transition& trans);
  bool _wasTransitionSuccessful(Transition trans);
  bool _checkState(ServerState state, bool debug=false);

};

#endif