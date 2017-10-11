#include "wifi_utils.h"

WiFiClass WiFi;

IPAddress::IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) {
}

IPAddress::IPAddress(){
}


wl_status_t WiFiClass::begin(const char* ssid, const char* password) {
  return WL_CONNECTED;
}

bool WiFiClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet){
  return true;
}


wl_status_t WiFiClass::status(){
  return WL_CONNECTED;
}

bool WiFiClass::disconnect(){
  return true;
}

int WiFiClass::scanNetworks() {
  return 1;
}

String WiFiClass::SSID(uint8_t) {
  return String("myssid");
}

void WiFiClass::mode(uint8_t m) {
}

WiFiClient::WiFiClient(){
}
int WiFiClient::read(){
  return 0;
}
size_t WiFiClient::write(uint8_t data){
  return 0;
}
size_t WiFiClient::write(const uint8_t *buf, size_t size){
  return 0;
}
void WiFiClient::stop(){
}
bool WiFiClient::available(){
  return true;
}
bool WiFiClient::connected(){
  return true;
}
WiFiClient::operator bool(){
  return true;
}

WiFiServer::WiFiServer(uint16_t port) {
}

void WiFiServer::begin(){
}

WiFiClient WiFiServer::available() { 
  return WiFiClient();
}
void WiFiServer::end() {

}
WiFiServer::operator bool(){
  return true;
}