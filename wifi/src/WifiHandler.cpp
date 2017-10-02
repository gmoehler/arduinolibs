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

WifiState WifiHandler::_determineNextState(){
  // currently we only go to the next in the chain (up or down)
  if (_targetState > _currentState){
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
  else if (_targetState < _currentState){
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
  return _currentState;
}

void WifiHandler::_invokeAction(){

  // upward
  if (_targetState > _currentState){
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

// check actions that happen asynchronously
bool WifiHandler::_checkState(WifiState state){

  // printf("Checking for state %d\n", state);
  switch(state){
    
    case DISCONNECTED:
    return (WiFi.status() != WL_CONNECTED);

    case CONNECTED:
    Serial.print(".");
    delay(500);
    return (WiFi.status() == WL_CONNECTED);
          
    case SERVER_LISTENING:
    return _server; // equals to is_listening()

    case CLIENT_CONNECTED:
    if (!_client) {
      _client = _server.available();
    }

    return _client.connected();

    case DATA_AVAILABLE:
    return _client.available();

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

  if (!_checkState(_currentState)){
    printf("Checking failed for state ");
    _printState(_currentState);
    // do something to recover
    return;
  }
  // Serial.print("Current state:");
  // _printState(_currentState);

  if (_currentState == _targetState){
    // target reached: nothing to be done here
    return;
  } 

  if (_currentState == _nextState){
    // reached next state, need to decide on a new action
    _nextState = _determineNextState();
    Serial.print("Next state: ");
    _printState(_nextState);
    _invokeAction();
  }

  if (_checkState(_nextState)){
    // reached next state
    Serial.print("NEW state:");
    _printState(_nextState);
    _currentState = _nextState;
  }

  delay(100);
}

