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

enum WifiState {  DISCONNECTED,
                  ERR_SSID_NOT_AVAIL,
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
  bool _invokeAction;
  Transition(WifiState f, WifiState t):from(f), to(t), _invokeAction(true){};
  bool operator==(Transition& rhs)const {
    return rhs.from == this->from && rhs.to == this->to;
  }
  Transition& operator=(const Transition& other) // copy assignment
  {
      if (this != &other) { // self-assignment check expected
          this->from = other.from;
          this->to = other.to;
          this->_invokeAction = other._invokeAction;
      }
      return *this;
  }
  bool withAction(){ return from != to;}
  void setInvokeAction(bool ia) {_invokeAction = ia;}
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
  
  Transition _determineNextTransition();
  Transition _determineDisconnectTransition();
  
  void _invokeAction(Transition& trans);
  bool _wasTransitionSuccessful(Transition trans);
  bool _checkState(WifiState state, bool debug=false);

  void _printState(WifiState state, bool withPrintln=false);
  void _printWiFiState( bool withPrintln=false);
  void _printTransition(Transition trans, bool withPrintln=false);
};

#endif