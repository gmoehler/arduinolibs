#include "RobustWiFiServer.h"

RobustWiFiServer::RobustWiFiServer():
  _currentTransition(DISCONNECTED,DISCONNECTED),
  _currentState(DISCONNECTED),
  _targetState(DISCONNECTED),
  _condition(NO_ERROR)
{};

const char * system_event_reasons[] = { "UNSPECIFIED", "AUTH_EXPIRE", "AUTH_LEAVE", "ASSOC_EXPIRE", "ASSOC_TOOMANY", "NOT_AUTHED", "NOT_ASSOCED", "ASSOC_LEAVE", "ASSOC_NOT_AUTHED", "DISASSOC_PWRCAP_BAD", "DISASSOC_SUPCHAN_BAD", "IE_INVALID", "MIC_FAILURE", "4WAY_HANDSHAKE_TIMEOUT", "GROUP_KEY_UPDATE_TIMEOUT", "IE_IN_4WAY_DIFFERS", "GROUP_CIPHER_INVALID", "PAIRWISE_CIPHER_INVALID", "AKMP_INVALID", "UNSUPP_RSN_IE_VERSION", "INVALID_RSN_IE_CAP", "802_1X_AUTH_FAILED", "CIPHER_SUITE_REJECTED", "BEACON_TIMEOUT", "NO_AP_FOUND", "AUTH_FAIL", "ASSOC_FAIL", "HANDSHAKE_TIMEOUT" };
#define reason2str(r) ((r>176)?system_event_reasons[r-176]:system_event_reasons[r-1])

void WiFiEvent(WiFiEvent_t event, system_event_info_t event_info)
{
  uint8_t reason;
  printf(" >>");
  switch (event) {
    case SYSTEM_EVENT_WIFI_READY:
      printf("Wifi ready.\n");
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      printf("Wifi scan done.\n");
      break;
    case SYSTEM_EVENT_STA_START:
      printf("Wifi started...\n");
      break;
    case SYSTEM_EVENT_STA_STOP:
      printf("Wifi disconnected.\n");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      printf("Wifi connected.\n");
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      printf("Authmode of Access Point has changed.\n");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      printf("IP address: %s\n", WiFi.localIP().toString().c_str());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("WiFi lost connection.\n");
      reason = event_info.disconnected.reason;
      printf("Reason: %u - %s\n", reason, reason2str(reason));
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      printf("WiFi connected by WPS.\n");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      printf("WPS connection failed.\n");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      printf("WPS connection timeout.\n");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      printf("WPS with pin mode: %s\n"/*, PIN2STR(event.event_info.sta_er_pin.pin_code)*/);
      break;
    default:
      printf("[WiFi-event] Unhandled event: %d\n", event);
      break;
  }

  printf("Wifi status:");
  switch (WiFi.status()){
    case WL_NO_SHIELD:
    printf("WL_NO_SHIELD");
    break;
    case WL_IDLE_STATUS:
    printf("WL_IDLE_STATUS");
    break;
    case WL_NO_SSID_AVAIL:
    printf("WL_NO_SSID_AVAIL");
    break;
    case WL_SCAN_COMPLETED:
    printf("WL_SCAN_COMPLETED");
    break;
    case WL_CONNECTED:
    printf("WL_CONNECTED");
    break;
    case WL_CONNECT_FAILED:
    printf("WL_CONNECT_FAILED");
    break;
    case WL_CONNECTION_LOST:
    printf("WL_CONNECTION_LOST");
    break;
    case WL_DISCONNECTED:
    printf("WL_DISCONNECTED");
    break;
    default:
    printf("Other status: %d", WiFi.status());
    break;
  }
  printf("\n");

  printf("mac: %s\n", WiFi.macAddress().c_str());
  printf("hostname: %s\n", WiFi.getHostname());
  printf("gateway: %s\n", WiFi.gatewayIP().toString().c_str());
}


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

   WiFi.onEvent(WiFiEvent);
  _server = WiFiServer(serverPort);

  }

void RobustWiFiServer::connect(){
  Serial.println("\nConnection request received...");
  _targetState = DATA_AVAILABLE;
}
  
void RobustWiFiServer:: disconnect(){
  Serial.println("\nDisconnect request received...");
  _targetState = DISCONNECTED;
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
  if (_targetState == DATA_AVAILABLE){
    return _determineNextConnectTransition();
  }
  return _determineNextDisconnectTransition();
}
  

Transition RobustWiFiServer::_determineNextConnectTransition(){
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

Transition RobustWiFiServer::_determineNextDisconnectTransition(){
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

Transition RobustWiFiServer::_getRevertTransition(Transition trans){
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
      // WiFi.disconnect();                    // to be on the safe side
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

  if (!stateok && debug) {
    _printInternalState();
  }
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
    _condition.error = STATE_CHECK_FAILED;
    Serial.print("ERROR. Checking failed for state ");
    printServerState(_currentState, true);

    if (_currentState == DISCONNECTED && WiFi.status() == WL_NO_SSID_AVAIL){
      // for SSID not found we have a special error state
      _currentState = ERR_SSID_NOT_AVAIL;
      _currentTransition = _determineNextTransition();
      printTransition(_currentTransition, true);
      delay(1000);
    }
    else {
      // switch back one state
      _currentTransition = _determineNextDisconnectTransition();
      printTransition(_currentTransition, true);
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

  // was in target state (with no action), but now have a different target
  else if (!_currentTransition.withAction() 
         && _currentState != _targetState) {
    _condition.resetError();
    _currentTransition = _determineNextTransition(); 
    Serial.println();
    Serial.print("NEW Transition: ");
    printTransition(_currentTransition, true);
  }

  // we stayed too long in this state, repeat action
  else if (_currentTransition.withAction() && _timeoutReached()){
    _condition.error = TRANSITION_TIMEOUT_REACHED;
    _condition.numberOfTimeouts++;
    Serial.println("Timeout reached. Will repeat last action.");
    // we first revert transition to be on the safe side
    // next iteration will automatically repeat action then
    _currentTransition = _getRevertTransition(_currentTransition);
  }

  // some actions are only invoked once
  _invokeAction(_currentTransition);
}

