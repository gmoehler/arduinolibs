#ifndef MOCK_WIFI_H
#define MOCK_WIFI_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "mock_Arduino.h"


typedef enum {
  WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
  WL_IDLE_STATUS      = 0,
  WL_NO_SSID_AVAIL    = 1,
  WL_SCAN_COMPLETED   = 2,
  WL_CONNECTED        = 3,
  WL_CONNECT_FAILED   = 4,
  WL_CONNECTION_LOST  = 5,
  WL_DISCONNECTED     = 6
} wl_status_t;

class IPAddress
{
public:
    IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    IPAddress();
};

#define WIFI_STA 1

class WiFiClass {
public:
  WiFiClass();
  wl_status_t begin(const char* ssid, const char* password);
  bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  bool disconnect();
  int scanNetworks();
  String SSID(uint8_t); 
  void mode(uint8_t m);

  wl_status_t status();

  void setStatus(wl_status_t status){_status = status;}
  void setNumSSIDs(int numSSIDs) {_numSSIDs = numSSIDs;}
  void setSSID(String ssid) {_ssid = ssid;}
private:
  wl_status_t _status;
  int _numSSIDs;
  String _ssid;
};

extern WiFiClass WiFi;

class WiFiClient
{
public:
  WiFiClient();
  int read();
  size_t write(uint8_t data);
  size_t write(const uint8_t *buf, size_t size) ;
  
  void stop();

  bool available();
  bool connected();
  operator bool();

  void setAvailable(bool avail){_available = avail;}
  void setConnected(bool conn){_connected = conn;}
  void printStatus();
private:
  bool _available;
  bool _connected;
};

class WiFiServer {
public:
  WiFiServer(uint16_t port=80);
  void begin();
  void end();
  
  WiFiClient available() ;
  operator bool();

  void clientConnected(bool cconn) {_available = cconn;}

  void setAvailable(bool avail){_available = avail;}
  void setListening(bool listening){_listening = listening;}
  void printStatus();
private:
  bool _available;
  bool _listening;
  WiFiClient _client;
};

#endif