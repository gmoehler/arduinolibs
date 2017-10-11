#ifndef WIFI_UTILS
#define WIFI_UTILS

#ifndef WITHIN_UNITTEST
  #include <WiFi.h>
#else
  #include "../test/mock_Arduino.h"
  #include "../test/mock_wifi.h"
#endif

enum ServerState {  
  DISCONNECTED,
  ERR_SSID_NOT_AVAIL,
  CONNECTED,
  SERVER_LISTENING,
  CLIENT_CONNECTED,
  DATA_AVAILABLE
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