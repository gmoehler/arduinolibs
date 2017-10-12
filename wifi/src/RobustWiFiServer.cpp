#include "RobustWiFiServer.h"

RobustWiFiServer::RobustWiFiServer():
  _currentTransition(DISCONNECTED,DISCONNECTED),
  _currentState(DISCONNECTED),
    _targetState(DISCONNECTED)
{};

void RobustWiFiServer::init(IPAddress ip, IPAddress gateway, IPAddress subnet, 
  uint16_t serverPort, String ssid, String wifiPassword) {
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

void RobustWiFiServer::setTargetState(ServerState targetState){
  _targetState = targetState;
}

ServerState RobustWiFiServer::getState() {
  return _currentState;
}

char RobustWiFiServer::readData(){
  return _client.read();             // read a byte
}

size_t RobustWiFiServer::writeData(uint8_t data){
  return _client.write(data);        // write data
}

size_t RobustWiFiServer::writeData(const uint8_t *buf, size_t size){
  return _client.write(buf, size);   // write data
}

Transition RobustWiFiServer::_determineNextTransition(){
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

Transition RobustWiFiServer::_determineDisconnectTransition(){
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

Transition RobustWiFiServer::_determineRevertTransition(Transition trans){
  return Transition(trans.to, trans.from);
}

void RobustWiFiServer::_invokeAction(Transition& trans){

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
      WiFi.begin(_ssid.c_str(), _wifiPassword.c_str());     // connect to router
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
      printServerState(trans.from);
      Serial.print(" --> ");
      printServerState(trans.to);
      Serial.println(".");
    }

    // remember invocation time for timeout
    trans.setLastInvocationTime();
    // invoke action only once
    trans._invokeAction = false;
  }
}

bool RobustWiFiServer::_wasTransitionSuccessful(Transition trans){
  // special checking when wifi connection was lost
  if (Transition(ERR_SSID_NOT_AVAIL, DISCONNECTED) == trans){
    Serial.println("Scanning SSIDs...");
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      Serial.println(WiFi.SSID(i).c_str());
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
bool RobustWiFiServer::_checkState(ServerState state, bool debug){

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
    printWiFiState(true);
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

//  if (!stateok && debug) {
    _printInternalState();
//  }
  return stateok;
}

void RobustWiFiServer::_printInternalState(){
	Serial.print(">> WiFi state: ");
    printWiFiState();
    Serial.print(" Server:");
    Serial.print(_server ? "conn " : "nc ");
    Serial.print(_server.available() ? "avail" : "na");
    Serial.print(" Client:");
    Serial.print(_client.connected() ? "conn " : "nc " );
    Serial.println(_client.available() ? "avail" : "na");
}

bool RobustWiFiServer::_timeoutReached(){
  uint32_t now = millis();
  return (now - _currentTransition.getLastInvocationTime() > 5000);
}

void RobustWiFiServer::loop(){

  // always verify current state to detect errors
  if (!_checkState(_currentState, true)
      && !_checkState(_currentTransition.to, true)){

    // current state has an error
    Serial.print("ERROR. Checking failed for state ");
    printServerState(_currentState, true);

    if (_currentState == DISCONNECTED && WiFi.status() == WL_NO_SSID_AVAIL){
      // for SSID not found we have a special state
      _currentState = ERR_SSID_NOT_AVAIL;
      _currentTransition = _determineNextTransition();
      printTransition(_currentTransition, true);
      delay(1000);
    }
    else {
      // switch back one state
      _currentTransition = _determineDisconnectTransition();
      printTransition(_currentTransition, true);
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
        printServerState(_targetState, true);
      }
    else {
      _currentTransition = _determineNextTransition();
    }
    Serial.println();
    Serial.print("NEW Transition: ");
    printTransition(_currentTransition, true);
  }

  // was in target state (with no action), but target has changed (also initially)
  else if (!_currentTransition.withAction() 
         && _currentState != _targetState) {
    _currentTransition = _determineNextTransition(); 
    Serial.println();
    Serial.print("NEW Transition: ");
    printTransition(_currentTransition, true);
  }
  else if (_timeoutReached()){
    Serial.println("Timeout reached. Will repeat last action.");
    _currentTransition = _determineRevertTransition(_currentTransition);
  }

  // some actions are only invoked once
  _invokeAction(_currentTransition);
}

