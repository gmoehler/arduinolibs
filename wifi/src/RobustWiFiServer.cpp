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

size_t WifiHandler::writeData(uint8_t data){
  return _client.write(data);        // write data
}

size_t WifiHandler::writeData(const uint8_t *buf, size_t size){
  return _client.write(buf, size);   // write data
}

Transition WifiHandler::_determineNextTransition(){
  switch(_currentState) {
    case DISCONNECTED:
    return Transition(DISCONNECTED, CONNECTED);

    case ERR_SSID_NOT_AVAIL:
    return Transition(ERR_SSID_NOT_AVAIL, DISCONNECTED);

    case CONNECTED:
    return Transition(CONNECTED,SERVER_LISTENING);

    case SERVER_LISTENING:
    return Transition(SERVER_LISTENING, CLIENT_CONNECTED);

    case CLIENT_CONNECTED:
    return Transition(CLIENT_CONNECTED, DATA_AVAILABLE);

    default: // no action when we are in DATA_AVAILABLE state
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

    default: // no action when we are in DISCONNECTED state
    return Transition( _currentState, _currentState);
  }
}

void WifiHandler::_invokeAction(Transition& trans){

  if (trans._invokeAction) {
    if (!trans.withAction()){
      // Serial.println("No action in transition...");
      // do nothing
    }
    // connecting actions...
    else if (Transition(DISCONNECTED, CONNECTED) == trans){
      Serial.println("Connecting to Wifi...");
      WiFi.mode(WIFI_STA);                  // "station" mode
      WiFi.disconnect();                    // to be on the safe side
      WiFi.config(_ip, _gateway, _subnet);  // set specific ip...
      WiFi.begin(_ssid, _wifiPassword);     // connect to router
    }
    else if (Transition(ERR_SSID_NOT_AVAIL, DISCONNECTED) == trans){
      // nothing to be done
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

    // disconnecting actions...
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
      Serial.print("ERROR. Unknown transition requested: ");
      _printState(trans.from);
      Serial.print(" --> ");
      _printState(trans.to);
      Serial.println(".");
    }

    trans.setLastInvocationTime();
    // invoke action only once
    trans._invokeAction = false;
  }
}

bool WifiHandler::_wasTransitionSuccessful(Transition trans){
  // special checking when wifi connection was lost
  if (Transition(ERR_SSID_NOT_AVAIL, DISCONNECTED) == trans){
    Serial.println("Scanning SSIDs...");
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i).equals( _ssid)){
        Serial.println("SSID found.");
        return true;
      }
      return false;
    }
  }
  return _checkState(trans.to);
}

// check whether a state that happen asynchronously
bool WifiHandler::_checkState(WifiState state, bool debug){

  bool stateok = false;
  switch(state){
    
    case DISCONNECTED:
    stateok = (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_NO_SSID_AVAIL);
    break;

    case ERR_SSID_NOT_AVAIL:
    stateok = (WiFi.status() == WL_NO_SSID_AVAIL);
    break;

    case CONNECTED:
    Serial.print(".");
    _printWiFiState(true);
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
    Serial.print(">> WiFi state: ");
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

void WifiHandler::_printState(WifiState state, bool withPrintln){
  switch(state){
    case DISCONNECTED:
    Serial.print("DISCONNECTED");
    break;
    case ERR_SSID_NOT_AVAIL:
    Serial.print("ERR_SSID_NOT_AVAIL");
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
  if (withPrintln){
    Serial.println();
  }
}

void WifiHandler::_printTransition(Transition trans, bool withPrintln){
  if (trans.withAction()) {
    _printState(trans.from);
    Serial.print(" --> ");
    _printState(trans.to);
  }
  else {
    Serial.print("No action");
  }
  if (withPrintln){
    Serial.println();
  }
}

void WifiHandler::_printWiFiState( bool withPrintln){
    
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
    Serial.print("WL_CONNECTION_LOST");
    break;
    case WL_DISCONNECTED:
    Serial.print("WL_DISCONNECTED");
    break;
    default:
    Serial.print("UNKNOWN WiFi STATE");
    break;
  } 
  if (withPrintln){
    Serial.println();
  }
}

void WifiHandler::loop(){

  // always verify current state to detect errors
  if (!_checkState(_currentState, true)
      && !_checkState(_currentTransition.to, true)){

    // current state has an error
    Serial.print("ERROR. Checking failed for state ");
    _printState(_currentState, true);

    if (_currentState == DISCONNECTED && WiFi.status() == WL_NO_SSID_AVAIL){
      // for SSID not found we have a special state
      _currentState = ERR_SSID_NOT_AVAIL;
      _currentTransition = _determineNextTransition();
      _printTransition(_currentTransition, true);
      delay(1000);
    }
    else {
      // switch back one state
      _currentTransition = _determineDisconnectTransition();
      _printTransition(_currentTransition, true);
      _currentState = _currentTransition.to;
      delay(200);
      }
  }

  // check whether transition was successful
  else if (_currentTransition.withAction() 
        && _wasTransitionSuccessful(_currentTransition)) {
    _currentState = _currentTransition.to;
    if (_currentState == _targetState) { 
        // create no-action transition
        _currentTransition = Transition(_currentState,_currentState);
        Serial.print("> Target reached: ");
        _printState(_targetState, true);
      }
    else {
      _currentTransition = _determineNextTransition();
    }
    Serial.println();
    Serial.print("NEW Transition: ");
    _printTransition(_currentTransition, true);
  }

  // was in target state (with no action), but target has changed (also initially)
  else if (!_currentTransition.withAction() 
         && _currentState != _targetState) {
    _currentTransition = _determineNextTransition(); 
    Serial.println();
    Serial.print("NEW Transition: ");
    _printTransition(_currentTransition, true);
  }

  // some actions are only invoked once
  _invokeAction(_currentTransition);
}

