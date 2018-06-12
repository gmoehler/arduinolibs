#include "RobustWiFiServer.h"

RobustWiFiServer::RobustWiFiServer():
  _currentTransition(DISCONNECTED,DISCONNECTED),
  _currentState(DISCONNECTED),
  _targetState(DISCONNECTED),
  _condition(NO_ERROR),
  _targetUpdated(false)
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

void RobustWiFiServer::connect(){
  LOGI(RWIFIS, "Connection request received...");
  _targetState = DATA_AVAILABLE;
  _targetUpdated = true;
}

void RobustWiFiServer::connect(IPAddress ip){
	// does not switch to new ip unless disconnected
	_ip = ip;
   connect();
}
  
void RobustWiFiServer:: disconnect(){
  LOGI(RWIFIS, "Disconnect request received...");
  _targetState = DISCONNECTED;
  _targetUpdated = true;
}

void RobustWiFiServer::clientDisconnect(){
	LOGI(RWIFIS, "Client disconnect request received...");
  _targetState = SERVER_LISTENING;
  _targetUpdated = true;
}

ServerState RobustWiFiServer::getState() {
  return _currentState;
}

ServerCondition RobustWiFiServer::getCondition(){
  return _condition;
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
  if (_targetState >= _currentState){
    return Transition(_currentState, getNextServerStateUp(_currentState));
  }
  return Transition(_currentState, getNextServerStateDown(_currentState));
}

Transition RobustWiFiServer::_getStepBackTransition(){
  ServerState fromState = _currentTransition.from;
  ServerState toState = (_targetState >= _currentState) ? 
    getNextServerStateDown(fromState) : 
    getNextServerStateUp(fromState);
  return Transition(fromState, toState);
}

void RobustWiFiServer::_invokeAction(Transition& trans){

  if (trans._invokeAction) {
    if (!trans.withAction()){
      LOGV(RWIFIS, "No action in transition...");
      // do nothing
    }
    // connecting actions...
    else if (Transition(DISCONNECTED, CONNECTED) == trans){
      LOGI(RWIFIS, "Connecting to Wifi with SSID %s & ip %s...", _ssid.c_str(), _ip.toString().c_str());
      WiFi.mode(WIFI_STA);                  // "station" mode
      // WiFi.disconnect();                    // causes auth error on following connect
      WiFi.config(_ip, _gateway, _subnet);  // set specific ip...
      WiFi.begin(_ssid.c_str(), _wifiPassword.c_str());     // connect to router
    }
    else if (Transition(ERR_SSID_NOT_AVAIL, DISCONNECTED) == trans){
      // nothing to be done
    }
    else if (Transition(CONNECTED, SERVER_LISTENING) == trans){
      LOGI(RWIFIS, "Starting server...");
      delay(500);
      _server.begin();                      // bind and listen
    }
    else if (Transition(SERVER_LISTENING, CLIENT_CONNECTED) == trans){
      LOGI(RWIFIS, "Listening for clients...");
      _client = _server.available();        // accept - also checked at checkState()
    }   
    else if (Transition(CLIENT_CONNECTED, DATA_AVAILABLE) == trans){
      LOGI(RWIFIS, "Waiting for data...");
      // nothing to be done
    } 

    // disconnecting actions...
    else if (Transition(DATA_AVAILABLE, CLIENT_CONNECTED) == trans){
      LOGD(RWIFIS, "Stop receiving data...");
      // nothing to be done
    }    
    else if (Transition(CLIENT_CONNECTED, SERVER_LISTENING) == trans){
      LOGI(RWIFIS, "Disconnect clients...");
      _client.stop();
    }
    else if (Transition(SERVER_LISTENING, CONNECTED) == trans){
      LOGI(RWIFIS, "Stopping server...");
      _server.end();
    }
    else if (Transition(CONNECTED, DISCONNECTED) == trans){
      LOGI(RWIFIS, "Disconnecting Wifi...");
      WiFi.disconnect();
    }
    // this transition appears only on no-ssid with reverted action
    // due to timeout
    else if (Transition(DISCONNECTED, ERR_SSID_NOT_AVAIL) == trans){
      // nothing to be done
    }
    else {
      LOGE(RWIFIS, "ERROR. Unknown transition requested: %s", trans.toString().c_str());
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
   LOGI(RWIFIS, "Scanning SSIDs for %s...", _ssid.c_str());
    int n = WiFi.scanNetworks(false, false, false, 500);
    bool foundSsid = false;
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i).equals( _ssid)){
        LOGI(RWIFIS, "*%s", WiFi.SSID(i).c_str());
        foundSsid = true;
      }
      else {
        LOGI(RWIFIS, "%s", WiFi.SSID(i).c_str());
      }
    }
    if (foundSsid) {
      LOGI(RWIFIS, "Specified SSID (%s) found...", _ssid.c_str());
      return true;
    }
    delay(2000);
    return false;
  }
  return _checkState(trans.to);
}

// check whether a state that happen asynchronously
bool RobustWiFiServer::_checkState(ServerState state, bool debug){

  bool stateok = false;
  LOGV(RWIFIS, "wifistate: %s", wiFiStateToString().c_str());
  switch(state){
    
    case ERR_SSID_NOT_AVAIL:
    stateok = (WiFi.status() == WL_NO_SSID_AVAIL);
    break;

    case DISCONNECTED:
    stateok = (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_NO_SSID_AVAIL);
    break;

    case CONNECTED:
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
    _printInternalState();
  }
  return stateok;
}

void RobustWiFiServer::_printInternalState(){
	LOGD(RWIFIS, "WiFi state: %s", wiFiStateToString().c_str());
  LOGD(RWIFIS, " Server %s %s", 
    _server ? "conn " : "nc ", _server.available() ? "avail" : "na");
  LOGD(RWIFIS, " Client: %s %s", 
    _client.connected() ? "conn " : "nc ", _client.available() ? "avail" : "na");
}

bool RobustWiFiServer::_timeoutReached(){
  // timeout does not make sense when ssid is not there
  if (_currentState == ERR_SSID_NOT_AVAIL) {
    return false;
  }
  uint32_t timeout = ( Transition(CONNECTED, SERVER_LISTENING) == _currentTransition ) ? 
      TRANSITION_TIMEOUT : LONG_TRANSITION_TIMEOUT;
  uint32_t now = millis();
  return (now - _currentTransition.getLastInvocationTime() > timeout);
}

void RobustWiFiServer::loop(){

  if (_targetUpdated) {
    _currentTransition = _determineNextTransition();
    _targetUpdated = false;
  }

  // always verify current state to detect errors
  // might already be in next state: check this
  if (!_checkState(_currentState, true)
      && !_checkState(_currentTransition.to, true)){

    // current state has an error
    _condition.error = STATE_CHECK_FAILED;
    LOGW(RWIFIS, "WARNING. Checking failed for state %s", serverStateToString(_currentState).c_str());

    if (_currentState == DISCONNECTED && WiFi.status() == WL_NO_SSID_AVAIL){
      // for SSID not found we have a special error state
      _currentState = ERR_SSID_NOT_AVAIL;
      _currentTransition = _determineNextTransition();
      LOGI(RWIFIS, "%s", _currentTransition.toString().c_str());
      delay(1000);
    }
    else {
      // switch back one state
      _currentTransition = _getStepBackTransition();
      LOGI(RWIFIS, "%s", _currentTransition.toString().c_str());
      _currentState = _currentTransition.to;
      delay(200);
    }
  }

  // check whether transition was successful
  else if (_currentTransition.withAction() 
           && _wasTransitionSuccessful(_currentTransition)) {
    _currentState = _currentTransition.to;
    _condition.resetError();
    if (_currentState == _targetState) { 
      // target reached: create no-action transition
      _currentTransition = Transition(_currentState,_currentState);
      LOGI(RWIFIS, "Target reached: %s", serverStateToString(_targetState).c_str());
    }
    else {
      _currentTransition = _determineNextTransition();
    }
    LOGD(RWIFIS, "NEW Transition: %s", _currentTransition.toString().c_str());
  }

  // when disconnected was targeted, ssid-not-avail is also fine (do this once only)
  else if (_currentState == ERR_SSID_NOT_AVAIL && _targetState == DISCONNECTED) {
    // target reached: create no-action transition once only
    if (_currentTransition.withAction()) {
      _currentTransition = Transition(_currentState,_currentState);
      LOGI(RWIFIS, "Disconnect target reached: %s", serverStateToString(_targetState).c_str());
      LOGD(RWIFIS, "NEW Transition: %s", _currentTransition.toString().c_str());
    }
  }

  // was in target state (with no action), but now have a different target
  else if (!_currentTransition.withAction() 
         && _currentState != _targetState) {
    _condition.resetError();
    _currentTransition = _determineNextTransition(); 
    LOGD(RWIFIS, "NEW Transition: %s", _currentTransition.toString().c_str());
  }

  // we stayed too long in this state, repeat action
  else if (_currentTransition.withAction() && _timeoutReached()){
    _condition.error = TRANSITION_TIMEOUT_REACHED;
    _condition.numberOfTimeouts++;
    // we first revert transition to be on the safe side
    // next iteration will automatically repeat action then
    _currentTransition = _getStepBackTransition();
    LOGI(RWIFIS, "Timeout reached. Will step back from %s to %s.", 
      serverStateToString(_currentTransition.from).c_str(),
      serverStateToString(_currentTransition.to).c_str());
  }

  // some actions are only invoked once
  _invokeAction(_currentTransition);
}
