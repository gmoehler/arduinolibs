#include "WifiHandler.h"

WifiHandler::WifiHandler(IPAddress ip, IPAddress gateway, IPAddress subnet, 
  uint16_t serverPort, char* ssid, char* wifiPassword):
  _ip(ip), _gateway(gateway), _subnet(subnet),
  _serverPort(serverPort), _ssid(ssid), _wifiPassword(wifiPassword),
  _targetState(DISCONNECTED), _currentState(DISCONNECTED)
  {};

void WifiHandler::setTargetState(WifiState targetState){
  _targetState = targetState;
}

WifiState WifiHandler::getState() {
  return _currentState;
}

void WifiHandler::loop(){
  if (_currentState == _targetState){
    return;
  } 
}
