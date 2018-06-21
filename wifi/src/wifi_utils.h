#ifndef WIFI_UTILS
#define WIFI_UTILS

#ifndef WITHIN_UNITTEST
  #include <Arduino.h>
  #include "esp_wifi.h"
  #include "esp_event_loop.h"
#else
  #include "../test/mock_Arduino.h"
  #include "../test/mock_wifi.h"
#endif

// define default compile time 
// #ifndef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
// #endif

#include "esp_log.h"

// need to define my own logging macros since other are not activated by LOG_LOCAL_LEVEL as expected
#ifndef LOG_MACROS
#define LOG_MACROS
#define LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { esp_log_write(ESP_LOG_ERROR,   tag, LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)    { esp_log_write(ESP_LOG_WARN,    tag, LOG_FORMAT(W, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)    { esp_log_write(ESP_LOG_INFO,    tag, LOG_FORMAT(I, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { esp_log_write(ESP_LOG_DEBUG,   tag, LOG_FORMAT(D, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { esp_log_write(ESP_LOG_VERBOSE, tag, LOG_FORMAT(V, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }

/* #define LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { printf(format "\n",  ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)    { printf(format "\n",  ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)    { printf(format "\n",  ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { printf(format "\n",  ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { printf(format "\n",  ##__VA_ARGS__); } */
#endif


#define RWIFIS  "RWIF"   // logging tag
#define WIFI_U  "WIFU"   // wifi utils

// for SSID connection only
enum WifiState {
  WIFI_UNCONFIGURED,
  WIFI_STARTED,
  WIFI_CONNECTED
} ;

extern WifiState wifiState;

enum ServerState {  
  UNCONFIGURED       = 0,
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
  bool _actionInvoked;
  uint32_t _lastInvocationTime;
  Transition(ServerState f, ServerState t):
    from(f), to(t), _actionInvoked(false), _lastInvocationTime(0){
      _lastInvocationTime = millis();
    };
  bool operator==(Transition& rhs)const {
    return rhs.from == this->from && rhs.to == this->to;
  }
  bool isEmptyTransition(){ return from == to;}
  void setActionInvoked(bool ai) {_actionInvoked = ai;}
  bool wasActionInvoked() {return _actionInvoked;}
  void setLastInvocationTime() {_lastInvocationTime = millis();}
  uint32_t getLastInvocationTime() { return _lastInvocationTime;}
  String  toString();
};

String serverStateToString(ServerState state);
String wiFiStateToString();
ServerState getNextServerStateUp(ServerState state);
ServerState getNextServerStateDown(ServerState state);
bool wifi_init();
bool wifi_start_sta(String ssid, String password, 
    IPAddress ip, IPAddress gateway, IPAddress subnet);
void wifi_stop_sta();
#endif