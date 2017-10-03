#include "WifiHandler.h"

// initialize static members
WifiState WifiHandler::_targetState = DISCONNECTED;
WifiState WifiHandler::_currentState = DISCONNECTED;
WifiState WifiHandler::_nextState = DISCONNECTED;

WiFiServer WifiHandler::_server;
WiFiClient WifiHandler::_client;

IPAddress WifiHandler::_ip; 
IPAddress WifiHandler::_gateway; 
IPAddress WifiHandler::_subnet; 
uint16_t WifiHandler::_serverPort;
char* WifiHandler::_ssid; 
char* WifiHandler::_wifiPassword;

bool WifiHandler::_errorSituation = false;


void WifiHandler::init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
  uint16_t serverPort, char* ssid, char* wifiPassword) {
  _ip = ip;
  _gateway = gateway;
  _subnet = subnet;
  _serverPort = serverPort;
  _ssid = ssid;
  _wifiPassword = wifiPassword;
  _targetState = DISCONNECTED; 
  _currentState = DISCONNECTED;

  _server = WiFiServer(serverPort);
  }

void WifiHandler::setTargetState(WifiState targetState){
  _targetState = targetState;
}

WifiState WifiHandler::getState() {
  return _currentState;
}

char WifiHandler::readData(){
  return _client.read();             // read a byte
}

WifiState WifiHandler::_determineNextState(bool upward){
  // currently we only go to the next in the chain (up or down)
  if (upward){
    switch(_currentState) {
      case DISCONNECTED:
      return CONNECTED;

      case CONNECTED:
      return SERVER_LISTENING;

      case SERVER_LISTENING:
      return CLIENT_CONNECTED;

      case CLIENT_CONNECTED:
      return DATA_AVAILABLE;

      default: // should not happen
      return _currentState;
    }
  }
  
  // downward
  switch(_currentState) {
    case DATA_AVAILABLE:
    return CLIENT_CONNECTED;

    case CLIENT_CONNECTED:
    return SERVER_LISTENING;

    case SERVER_LISTENING:
    return CONNECTED;

    case CONNECTED:
    return DISCONNECTED;

    default: // should not happen
    return _currentState;
  }

}

void WifiHandler::_invokeAction(bool upward){
  if (upward){
    // since we only hop by one state we only need to check the _nextState
    switch(_nextState) {
      case CONNECTED:
      Serial.println("Connecting to Wifi...");
      WiFi.mode(WIFI_STA);                  // "station" mode
      WiFi.disconnect();                    // to be on the safe side
      WiFi.config(_ip, _gateway, _subnet);  // set specific ip...
      WiFi.begin(_ssid, _wifiPassword);     // connect to router
      break;

      case SERVER_LISTENING:
      Serial.println("Starting server...");
      delay(500);
      _server.begin();                      // bind and listen
      break;

      case CLIENT_CONNECTED:
      Serial.println("Listening for clients...");
      _client = _server.available();        // accept - also checked at checkState()
      break;

      case DATA_AVAILABLE:
      Serial.println("Waiting for data...");
      break;                                // done in checkState()

      default:                              // should not happen
      break;
    }
  }

  // downward
  else {
    switch(_nextState) {
      case CLIENT_CONNECTED:
      Serial.println("Stop receiving data...");
      break;                               

      case SERVER_LISTENING:
      Serial.println("Stop listening for clients...");
      _client.stop();
      break;

      case CONNECTED:
      Serial.println("Stopping server...");
      _server.end();
      break;

      case DISCONNECTED:
      Serial.println("Disconnecting Wifi...");
      WiFi.disconnect();
      break;

      default:                         // should not happen
      break;
    }
  }
}

// check whether a state that happen asynchronously
bool WifiHandler::_checkState(WifiState state){

  // printf("Checking for state %d\n", state);
  switch(state){
    
    case DISCONNECTED:
    return (WiFi.status() != WL_CONNECTED);

    case CONNECTED:
    Serial.print(".");
    Serial.print(WiFi.status());
    delay(500);
    return (WiFi.status() == WL_CONNECTED);
          
    case SERVER_LISTENING:
    // need to also check wifi status since server would not notice failing wifi
    return (WiFi.status() == WL_CONNECTED) && _server; // equals to is_listening()

    case CLIENT_CONNECTED:
    if (!_client) {
      _client = _server.available();
    }
    return  (WiFi.status() == WL_CONNECTED) && _client.connected();

    case DATA_AVAILABLE:
    return  (WiFi.status() == WL_CONNECTED) && _client.available();

    default: // unknown state
    return false;
  } 
}

void WifiHandler::_printState(WifiState state){
  switch(state){
    
    case DISCONNECTED:
    Serial.println("DISCONNECTED.");
    break;

    case CONNECTED:
    Serial.println("CONNECTED.");
    break;
          
    case SERVER_LISTENING:
    Serial.println("SERVER_LISTENING.");
    break;

    case CLIENT_CONNECTED:
    Serial.println("CLIENT_CONNECTED.");
    break;

    case DATA_AVAILABLE:
    Serial.println("DATA_AVAILABLE.");
    break;

    default:
    Serial.println("UNKNOWN_STATE.");
    break;
  } 
}


void WifiHandler::loop(){

  // always verify current state
  if (!_checkState(_currentState)){
    Serial.print("Checking failed for state ");
    _printState(_currentState);

    // stepping one down was not enough
    // go one step further
    if (_errorSituation){
      _currentState = _nextState;
    }
    _errorSituation = true;

    // switch back one state
    bool upward = false;
    _nextState = _determineNextState(upward);
    Serial.print("Next state: ");
    _printState(_nextState);
    _invokeAction(upward);
  }

  // Serial.print("Current state:");
  // _printState(_currentState);

  if (_currentState == _nextState){
    // reached next state
    if (_currentState == _targetState){
      // target reached: nothing to be done here
      return;
    }   
    // not target state: need to decide on a new action
    bool upward = _targetState > _currentState;
    _nextState = _determineNextState(upward);
    Serial.print("Next state: ");
    _printState(_nextState);
    _invokeAction(upward);
  }

  // special handling when ssid is not available
  // TODO: find a better place for this (additional state?)
  if (_nextState == CONNECTED && 
        (WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_DISCONNECTED)) {
    // need to call WiFi.begin() again
    _invokeAction(true);
  }

  if (_checkState(_nextState)){
    // reached next state
    Serial.print("NEW state:  ");
    _printState(_nextState);
    _currentState = _nextState;
    _errorSituation = false;
  } else [
    // count retries and repeat action if it takes too long to succeed
    // e.g. server does not start after wifi is just back and not stable yet
  ]

  delay(100);
}

