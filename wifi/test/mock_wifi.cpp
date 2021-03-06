#include "wifi_utils.h"

WiFiClass WiFi;

IPAddress::IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) {
}

IPAddress::IPAddress(){
}

/*********************************/
WiFiClass::WiFiClass()
  : _status(WL_DISCONNECTED) {
  }

wl_status_t WiFiClass::begin(const char* ssid, const char* password) {
  _ssid = String(ssid);
  if (_status != WL_NO_SSID_AVAIL){
    _status = WL_CONNECTED;
  } // else ssid is not there
  return _status;
}

bool WiFiClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet){
  return true;
}

wl_status_t WiFiClass::status(){
  return _status;
}

bool WiFiClass::disconnect(){
  _status = WL_DISCONNECTED;
  return true;
}

int WiFiClass::scanNetworks() {
  return _numSSIDs;
}

String WiFiClass::SSID(uint8_t) {
  return _ssid;
}

void WiFiClass::mode(uint8_t m) {
}

/*********************************/

WiFiClient::WiFiClient()
  : _available(false), _connected(false){
}
int WiFiClient::read(){
  _available = false; // only read one byte
  return 1;
}
size_t WiFiClient::write(uint8_t data){
  return 1;
}
size_t WiFiClient::write(const uint8_t *buf, size_t size){
  return 1;
}
void WiFiClient::stop(){
  _connected = false;
}
bool WiFiClient::available(){
  return _available;
}
bool WiFiClient::connected(){
  return _connected;
}
WiFiClient::operator bool(){
  return connected();
}

void WiFiClient::printStatus(){
  printf("Client:");
  if( _connected){
  	printf(" connected");
  }
  if( _available){
  	printf(" data_avail");
  }
  printf("\n");
}

/*********************************/

WiFiServer::WiFiServer(uint16_t port)
  : _available(false), _listening(false) {
}

void WiFiServer::begin(){
  _listening = true;
  _available = false;
}

void WiFiServer::end() {
  _listening = false;
  _available = false;
}

WiFiClient WiFiServer::available() { 
  _client.setConnected(_available);
  return _client;
}

WiFiServer::operator bool(){
	//printf("server bool %d\n", _listening?1:0);
  return _listening;
}

void WiFiServer::printStatus(){
  printf("Server:");
  if( _listening){
  	printf(" listening");
  }
  if( _available){
  	printf(" client_avail");
  }
  printf("\n");
}

