#include "test.h"
#include <RobustWiFiServer.h>


void prepareState(ServerState state, RobustWiFiServer& rs) {
  WiFiClient& c = rs._getClient();
  WiFiServer& s = rs._getServer();

  switch(state){
    case DISCONNECTED:
    WiFi.setStatus(WL_DISCONNECTED);
    s.setAvailable(false);
    s.setListening(false);
    c.setAvailable(false);
    c.setConnected(false);
    break;

    case ERR_SSID_NOT_AVAIL:
    WiFi.setStatus(WL_NO_SSID_AVAIL);
    s.setAvailable(false);
    s.setListening(false);
    c.setAvailable(false);
    c.setConnected(false);
    break;

    case CONNECTED:
    WiFi.setStatus(WL_CONNECTED);
    s.setAvailable(false);
    s.setListening(false);
    c.setAvailable(false);
    c.setConnected(false);
    break;

    case SERVER_LISTENING:
    WiFi.setStatus(WL_CONNECTED);
    s.setAvailable(false);
    s.setListening(true);
    c.setAvailable(false);
    c.setConnected(false);
    break;

    case CLIENT_CONNECTED:
    WiFi.setStatus(WL_CONNECTED);
    s.setAvailable(true);
    s.setListening(true);
    c.setAvailable(false);
    c.setConnected(true);
    break;

    case DATA_AVAILABLE:
    WiFi.setStatus(WL_CONNECTED);
    s.setAvailable(true);
    s.setListening(true);
    c.setAvailable(true);
    c.setConnected(true);
    break;
    
    default:
    WiFi.setStatus(WL_DISCONNECTED);
    s.setAvailable(false);
    s.setListening(false);
    c.setAvailable(false);
    c.setConnected(false);
    break;
  }
   printf("test>");
   rs._printInternalState();
}

TEST(StaticHandler, runthru){
  const String ssid     = "MY_SSID";
  const String password = "my_password";
  
  IPAddress myIP(192,168,1,127);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  uint16_t port = 1110;

  RobustWiFiServer wifiServer;

  wifiServer.init(myIP, gateway, subnet, port, ssid, password);
  wifiServer.setTargetState(DATA_AVAILABLE);
  prepareState(DISCONNECTED, wifiServer);
  EXPECT_EQ( wifiServer.getState(), DISCONNECTED);

  for (int i=0; i< 4; i++){

    wifiServer.loop();
    
    ServerState state = wifiServer.getState();
    switch(i){
      case 0:
      EXPECT_EQ(state, DISCONNECTED);
      prepareState(CONNECTED, wifiServer);
      break;
    
      case 1:
      EXPECT_EQ(state, CONNECTED);
      prepareState(SERVER_LISTENING, wifiServer);
      break;

      case 2:
      EXPECT_EQ(state, SERVER_LISTENING);
      prepareState(CLIENT_CONNECTED, wifiServer);
      break;

      case 3:
      EXPECT_EQ(state, CLIENT_CONNECTED);
      prepareState(DATA_AVAILABLE, wifiServer);
      break;

      case 4:
      EXPECT_EQ(state, DATA_AVAILABLE);
      break;
    }
    
  }
}
