#ifndef WIFI_HANDLER
#define WIFI_HANDLER

/**
 * Robust wifi handler
 * 
 * Can handle the following wifi situations:
 * 1. client connects
 * 2. client sends data
 * 3. client disconnects when idle
 * 4. client is interrupted during transmission
 * 5. wifi stops and is restarted when no client is connected
 * 6. wifi stops and is restarted when client is connected (not tested yet)
 * 7. wifi stops during client transmission (still issues with that)
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
  bool withAction(){ return from != to;}
};

class WifiHandler
{
public:
  WifiHandler();
  void init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
    uint16_t serverPort, char* ssid, char* wifiPassword); // to be called in setup()

  void setTargetState(WifiState targetState);
  WifiState getState();

  char readData();  // read data (in state DATA_AVAILABLE)

  void loop();      // to be called in loop()

private:
  Transition _currentTransition; // current state transition
  WifiState _currentState; // current state
  WifiState _targetState;  // ultimate target

  IPAddress _ip; 
  IPAddress _gateway; 
  IPAddress _subnet; 
  uint16_t _serverPort;
  char* _ssid; 
  char* _wifiPassword;

  WiFiServer _server;
  WiFiClient _client;
  
  Transition _determineConnectTransition();
  Transition _determineDisconnectTransition();
  
  void _invokeAction(Transition& trans);
  bool _transitionSuccessful(Transition trans);
  bool _checkState(WifiState state, bool debug=false);

  void _printState(WifiState state);
  void _printWiFiState();
  void _printTransition(Transition trans);
};

#endif