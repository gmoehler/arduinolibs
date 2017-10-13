#ifndef WIFI_UTILS
#define WIFI_UTILS

#ifndef WITHIN_UNITTEST
  #include <WiFi.h>
#else
  #include "../test/mock_Arduino.h"
  #include "../test/mock_wifi.h"
#endif

enum ServerState {  
  DISCONNECTED       = 0,
  CONNECTED          = 1,
  SERVER_LISTENING   = 2,
  CLIENT_CONNECTED   = 3,
  DATA_AVAILABLE     = 4,
  ERR_SSID_NOT_AVAIL = 90,
  UNKNOWN            = 99
};

enum ServerError {
  NO_ERROR,
  STATE_CHECK_FAILED,
  TRANSITION_TIMEOUT_REACHED
};

class ServerCondition {
public:
  ServerError error;
  uint8_t numberOfTimeouts;
  ServerCondition(ServerError err);
  void resetError();    
};

class Transition
{ 
public:
  ServerState from;
  ServerState to;
  bool _invokeAction;
  uint32_t _lastInvocationTime;
  Transition(ServerState f, ServerState t):
    from(f), to(t), _invokeAction(true), _lastInvocationTime(0){};
  bool operator==(Transition& rhs)const {
    return rhs.from == this->from && rhs.to == this->to;
  }
  bool withAction(){ return from != to;}
  void setInvokeAction(bool ia) {_invokeAction = ia;}
  void setLastInvocationTime() {_lastInvocationTime = millis();}
  uint32_t getLastInvocationTime() { return _lastInvocationTime;}
};

void printWiFiState(bool withPrintln=false);
void printServerState(ServerState state, bool withPrintln=false);
void printTransition(Transition trans, bool withPrintln=false);

#endif