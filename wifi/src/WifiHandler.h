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

#include <utility>
#include <WiFi.h>

enum WifiState {  DISCONNECTED,
                  CONNECTED,
                  SERVER_LISTENING,
                  CLIENT_CONNECTED,
                  DATA_AVAILABLE
};

class Transition
{
public:
  WifiState from;
  WifiState to;
  bool wasInvoked;
  Transition(WifiState f, WifiState t):from(f), to(t), wasInvoked(false){};
  bool operator==(Transition& rhs)const {
    return rhs.from == this->from && rhs.to == this->to;
  }
  Transition& operator=(const Transition& other) // copy assignment
  {
      if (this != &other) { // self-assignment check expected
          this->from = other.from;
          this->to = other.to;
          this->wasInvoked = other.wasInvoked;
      }
      return *this;
  }
  bool needToPerformAction(){ return from != to;}
};

class WifiHandler
{
public:
  WifiHandler();
  void init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
    uint16_t serverPort, char* ssid, char* wifiPassword);

  void setTargetState(WifiState targetState);
  WifiState getState();
  char readData();

  void loop();

private:
  Transition _currentTransition; 
  WifiState _currentState; // current state
  WifiState _targetState;  // ultimate target

  WiFiServer _server;
  WiFiClient _client;

  IPAddress _ip; 
  IPAddress _gateway; 
  IPAddress _subnet; 
  uint16_t _serverPort;
  char* _ssid; 
  char* _wifiPassword;

  Transition _determineConnectTransition();
  Transition _determineErrorTransition();
  
  void _invokeAction(Transition trans);
  bool _transitionSuccessful(Transition trans);
  bool _checkState(WifiState state, bool printStatus=false);

  void _printState(WifiState state);
  void _printWiFiState();
  void _printTransition(Transition trans);

  bool _errorSituation;

};

#endif