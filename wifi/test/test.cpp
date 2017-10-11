#include "test.h"
#include <RobustWiFiServer.h>


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
  EXPECT_EQ( wifiServer.getState(), DISCONNECTED);

  for (int i=0; i< 3; i++){
    wifiServer.loop();
    ServerState state = wifiServer.getState();
    switch(i){
      case 0:
      EXPECT_EQ(state, CONNECTED);
      break;

      case 1:
      EXPECT_EQ(state, SERVER_LISTENING);
      break;

      case 2:
      EXPECT_EQ(state, CLIENT_CONNECTED);
      break;

      case 3:
      EXPECT_EQ(state, DATA_AVAILABLE);
      break;

    }
    
  }
}
