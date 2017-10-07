#include "WifiHandler.h"

WifiHandler::WifiHandler():
  _currentTransition(DISCONNECTED,DISCONNECTED),
  _targetState(DISCONNECTED),
  _currentState(DISCONNECTED)
{};

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

Transition WifiHandler::_determineNextTransition(){
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

Transition WifiHandler::_determineDisconnectTransition(){
  switch(_currentState) {
    case DATA_AVAILABLE:
    return Transition(DATA_AVAILABLE, CLIENT_CONNECTED);

    case CLIENT_CONNECTED:
    return Transition(CLIENT_CONNECTED, SERVER_LISTENING);

    case SERVER_LISTENING:
    return Transition(SERVER_LISTENING, CONNECTED);

    case CONNECTED:
    return Transition(CONNECTED, DISCONNECTED);

    default: // should not happen
    return Transition( _currentState, _currentState);
  }
}

void WifiHandler::_invokeAction(Transition& trans){

  if (trans.wasInvoked) {
    // invoke only once
    // Serial.println("Action was already invoked.");
  }
  else {
    if (!trans.withAction()){
      // Serial.println("No action in transition...");
      // do nothing
    }
    // connecting...
    else if (Transition(DISCONNECTED, CONNECTED) == trans){
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
      // nothing to be done
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

    // invoke only once
    trans.wasInvoked = true;
  }

}

bool WifiHandler::_transitionSuccessful(Transition trans){
  return _checkState(trans.to);
}

// check whether a state that happen asynchronously
bool WifiHandler::_checkState(WifiState state, bool debug){

  bool stateok = false;
  switch(state){
    
    case DISCONNECTED:
    stateok = (WiFi.status() != WL_CONNECTED);
    break;

    case CONNECTED:
    Serial.print(".");
    _printWiFiState();
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

  if (!stateok && debug) {
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
    Serial.print("DISCONNECTED");
    break;

    case CONNECTED:
    Serial.print("CONNECTED");
    break;
          
    case SERVER_LISTENING:
    Serial.print("SERVER_LISTENING");
    break;

    case CLIENT_CONNECTED:
    Serial.print("CLIENT_CONNECTED");
    break;

    case DATA_AVAILABLE:
    Serial.print("DATA_AVAILABLE");
    break;

    default:
    Serial.print("UNKNOWN_STATE");
    break;
  } 
}

void WifiHandler::_printTransition(Transition trans){
  if (trans.withAction()) {
    _printState(trans.from);
    Serial.print(" --> ");
    _printState(trans.to);
    Serial.println(".");
  }
  else {
    Serial.print("No action");
  }
}

void WifiHandler::_printWiFiState(){
    
  wl_status_t wifiState = WiFi.status();
  
  switch(wifiState){

    case WL_NO_SHIELD:
    Serial.print("WL_NO_SHIELD");
    break;

    case WL_IDLE_STATUS:
    Serial.print("WL_IDLE_STATUS");
    break;
          
    case WL_NO_SSID_AVAIL:
    Serial.print("WL_NO_SSID_AVAIL");
    break;

    case WL_SCAN_COMPLETED:
    Serial.print("WL_SCAN_COMPLETED");
    break;

    case WL_CONNECTED:
    Serial.print("WL_CONNECTED");
    break;

    case WL_CONNECT_FAILED:
    Serial.print("WL_CONNECT_FAILED");
    break;

    case WL_CONNECTION_LOST:
    Serial.print("WL_CONNECTION_LOST.");
    break;

    case WL_DISCONNECTED:
    Serial.print("WL_DISCONNECTED");
    break;

    default:
    Serial.print("UNKNOWN WiFi STATE");
    break;
  } 
}


void WifiHandler::loop(){

  // always verify current state
  if (!_checkState(_currentState)){
    // current state has error
    Serial.print("! Checking failed for state ");
    _printState(_currentState);
    Serial.println();

    // switch back one state
    _currentTransition = _determineDisconnectTransition();
    _printTransition(_currentTransition);
    _currentState = _currentTransition.to;
  }

  // check whether transition was successful
  else if (_currentTransition.withAction() 
            && _transitionSuccessful(_currentTransition)) {
    _currentState = _currentTransition.to;
    if (_currentState == _targetState) { 
        // no-action transition
        _currentTransition = Transition(_currentState,_currentState);
        Serial.println("> Target reached:");
        _printState(_targetState);
      }
    else {
      _currentTransition = _determineNextTransition();
    }
    Serial.println();
    Serial.print("NEW Transition: ");
    _printTransition(_currentTransition);
  }

  // was in target state (with no action), but target has changed (also initially)
  else if (!_currentTransition.withAction() 
            && _currentState != _targetState) {
    _currentTransition = _determineNextTransition(); 
    Serial.println();
    Serial.print("NEW Transition: ");
    _printTransition(_currentTransition);
  }

  // only invoked once
  _invokeAction(_currentTransition);

  // delay(100);
}

