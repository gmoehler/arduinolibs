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

Transition WifiHandler::_determineConnectTransition(){
  switch(_currentState) {
    case DISCONNECTED:
    return Transition(DISCONNECTED, CONNECTED);

    case CONNECTED:
    return Transition(CONNECTED,SERVER_LISTENING);

    case SERVER_LISTENING:
    return Transition(SERVER_LISTENING, CLIENT_CONNECTED);

    case CLIENT_CONNECTED:
    return Transition(CLIENT_CONNECTED, DATA_AVAILABLE);

    default: // should not happen
    return Transition(_currentState,_currentState);
  }

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

void WifiHandler::_invokeAction(Transition trans){

  // connecting...
  if (Transition(DISCONNECTED, CONNECTED) == trans){
    Serial.println("Connecting to Wifi...");
    WiFi.mode(WIFI_STA);                  // "station" mode
    WiFi.disconnect();                    // to be on the safe side
    WiFi.config(_ip, _gateway, _subnet);  // set specific ip...
    WiFi.begin(_ssid, _wifiPassword);     // connect to router
  }
  else if (Transition(CONNECTED, SERVER_LISTENING) == trans){
    Serial.println("Starting server...");
    delay(500);
    _server.begin();                      // bind and listen
  }
  else if (Transition(SERVER_LISTENING, CLIENT_CONNECTED) == trans){
    Serial.println("Listening for clients...");
    _client = _server.available();        // accept - also checked at checkState()
  }   
  else if (Transition(CLIENT_CONNECTED, DATA_AVAILABLE) == trans){
    Serial.println("Waiting for data...");
  } 

  // disconnecting...
  else if (Transition(DATA_AVAILABLE, CLIENT_CONNECTED) == trans){
    Serial.println("Stop receiving data...");
  }    
  else if (Transition(CLIENT_CONNECTED, SERVER_LISTENING) == trans){
    Serial.println("Disconnect clients...");
    _client.stop();
  }
  else if (Transition(SERVER_LISTENING, CONNECTED) == trans){
    Serial.println("Stopping server...");
    _server.end();
  }
  else if (Transition(CONNECTED, DISCONNECTED) == trans){
    Serial.println("Disconnecting Wifi...");
    WiFi.disconnect();
  }
  else {
    Serial.print("Error: Unknown transition requested: ");
    _printState(trans.from);
    Serial.print(" --> ");
    _printState(trans.to);
    Serial.println(".");
  }
}

bool WifiHandler::_transitionSuccessfull(Transition trans){
  return _checkState(trans.to);
}

// check whether a state that happen asynchronously
bool WifiHandler::_checkState(WifiState state, bool printStatus){

  bool stateok = false;
  switch(state){
    
    case DISCONNECTED:
    stateok = (WiFi.status() != WL_CONNECTED);
    break;

    case CONNECTED:
    Serial.print(".");
    Serial.print(WiFi.status());
    delay(500);
    stateok = (WiFi.status() == WL_CONNECTED);
    break;

    case SERVER_LISTENING:
    // need to also check wifi status since server would not notice failing wifi
    stateok = (WiFi.status() == WL_CONNECTED) && _server; // equals to is_listening()
    break;

    case CLIENT_CONNECTED:
    if (!_client) {
      _client = _server.available();
    }
    stateok =  (WiFi.status() == WL_CONNECTED) && _client.connected();
    break;

    case DATA_AVAILABLE:
    stateok =  (WiFi.status() == WL_CONNECTED) && _client.available();
    break;

    default: // unknown state
    stateok = false;
    break;
  } 

  if (!stateok && printStatus) {
    Serial.print(">> WiFi state:");
    _printWiFiState();
    Serial.print(" Server:");
    Serial.print(_server ? "conn" : "nc");
    Serial.print(" Client:");
    Serial.print(_client.connected() ? "conn" : "nc");
    Serial.print(" Client data:");
    Serial.println(_client.available() ? "avail" : "na");
  }
  return stateok;
}

void WifiHandler::_printState(WifiState state){
  switch(state){
    
    case DISCONNECTED:
    Serial.print("DISCONNECTED.");
    break;

    case CONNECTED:
    Serial.print("CONNECTED.");
    break;
          
    case SERVER_LISTENING:
    Serial.print("SERVER_LISTENING.");
    break;

    case CLIENT_CONNECTED:
    Serial.print("CLIENT_CONNECTED.");
    break;

    case DATA_AVAILABLE:
    Serial.print("DATA_AVAILABLE.");
    break;

    default:
    Serial.print("UNKNOWN_STATE.");
    break;
  } 
}

void WifiHandler::_printTransition(Transition trans){
  _printState(trans.from);
  Serial.print(" --> ");
  _printState(trans.to);
  Serial.println(".");
}

void WifiHandler::_printWiFiState(){
    
  wl_status_t wifiState = WiFi.status();
  
  switch(wifiState){

    case WL_NO_SHIELD:
    Serial.print("WL_NO_SHIELD.");
    break;

    case WL_IDLE_STATUS:
    Serial.print("WL_IDLE_STATUS.");
    break;
          
    case WL_NO_SSID_AVAIL:
    Serial.print("WL_NO_SSID_AVAIL.");
    break;

    case WL_SCAN_COMPLETED:
    Serial.print("WL_SCAN_COMPLETED.");
    break;

    case WL_CONNECTED:
    Serial.print("WL_CONNECTED.");
    break;

    case WL_CONNECT_FAILED:
    Serial.print("WL_CONNECT_FAILED.");
    break;

    case WL_CONNECTION_LOST:
    Serial.print("WL_CONNECTION_LOST.");
    break;

    case WL_DISCONNECTED:
    Serial.print("WL_DISCONNECTED.");
    break;

    default:
    Serial.print("UNKNOWN WiFi STATE.");
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
    _invokeAction(Transition(_currentState, _nextState));
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
    //TODO: only works for connection target
    Transition trans = _determineConnectTransition();
    _nextState = trans.to;
    _printTransition(trans);
    _invokeAction(trans);
  }

  // special handling when ssid is not available
  // TODO: find a better place for this (additional state?)
  if (_nextState == CONNECTED && 
        (WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_DISCONNECTED)) {
    // need to call WiFi.begin() again
    //_invokeAction(Transition(_currentState, _nextState))
  }

  if (_checkState(_nextState)){
    // reached next state
    Serial.print("NEW state:  ");
    _printState(_nextState);
    _currentState = _nextState;
    _errorSituation = false;
  } else {
    // count retries and repeat action if it takes too long to succeed
    // e.g. server does not start after wifi is just back and not stable yet
  }

  delay(100);
}

