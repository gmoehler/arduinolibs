#ifndef WIFI_UTILS
#define WIFI_UTILS

#ifndef WITHIN_UNITTEST
  #include <WiFi.h>
#else
  #include "../test/mock_Arduino.h"
  #include "../test/mock_wifi.h"
#endif

// define default compile time 
// #ifndef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
// #endif

#include "esp_log.h"

// need to define my own logging macros since other are not activated by LOG_LOCAL_LEVEL as expected
#ifndef LOG_MACROS
#define LOG_MACROS
#define LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { printf(format "\n",  ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)    { printf(format "\n",  ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)    { printf(format "\n",  ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { printf(format "\n",  ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { printf(format "\n",  ##__VA_ARGS__); }
#endif


#define RWIFIS  "RWIF"   // logging tag

enum ServerState {  
  ERR_SSID_NOT_AVAIL = 0,
  DISCONNECTED       = 1,
  CONNECTED          = 2,
  SERVER_LISTENING   = 3,
  CLIENT_CONNECTED   = 4,
  DATA_AVAILABLE     = 5,
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
  String  toString();
};

String serverStateToString(ServerState state);
String wiFiStateToString();
void onWiFiEvent(WiFiEvent_t event, system_event_info_t event_info);
ServerState getNextServerStateUp(ServerState state);
ServerState getNextServerStateDown(ServerState state);
#endif