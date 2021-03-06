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
   printf("test");
   rs._printInternalState();
}

void client_connect(RobustWiFiServer& rs) {
  printf("test> client connects.\n");
  prepareState(CLIENT_CONNECTED, rs);
}

void client_disconnects(RobustWiFiServer& rs) {
  printf("test> client disconnects.\n");
  prepareState(SERVER_LISTENING, rs);
}

void client_send_data(RobustWiFiServer& rs) {
  printf("test> client sends data.\n");
  prepareState(DATA_AVAILABLE, rs);
}

void wifi_disconnects(RobustWiFiServer& rs) {
  printf("test> wifi disconnects.\n");
  prepareState(ERR_SSID_NOT_AVAIL, rs);
}


const String ssid     = "MY_SSID";
const String password = "my_password";

IPAddress myIP(192,168,1,127);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

uint16_t port = 1110;

TEST(StaticHandler, runthruWithWifiDisconnect){

  RobustWiFiServer wifiServer;

  wifiServer.init(myIP, gateway, subnet, port, ssid, password);
  wifiServer.connect();
  prepareState(DISCONNECTED, wifiServer);
  EXPECT_EQ( wifiServer.getState(), DISCONNECTED);

  // protocol:
  // connects ordinary
  // client disconnects
  // wifi breaks down
  // wifi back
  // connect ordinary
  for (int i=0; i<=15; i++){
    printf("%d\n", i);

    wifiServer.loop();
    ServerCondition cond = wifiServer.getCondition();
    
    ServerState state = wifiServer.getState();
    switch(i){
      case 0:
      EXPECT_EQ(state, DISCONNECTED);
      EXPECT_EQ(cond.error, NO_ERROR);
      //prepareState(CONNECTED, wifiServer);
      break;
    
      case 1:
      EXPECT_EQ(state, CONNECTED);
      EXPECT_EQ(cond.error, NO_ERROR);
      //prepareState(SERVER_LISTENING, wifiServer);
      break;

      case 2:
      EXPECT_EQ(state, SERVER_LISTENING);
      EXPECT_EQ(cond.error, NO_ERROR);
      client_connect(wifiServer);
      break;

      case 3:
      EXPECT_EQ(state, CLIENT_CONNECTED);
      EXPECT_EQ(cond.error, NO_ERROR);
      client_send_data(wifiServer);
      break;

      case 4:
      EXPECT_EQ(state, DATA_AVAILABLE);
      EXPECT_EQ(cond.error, NO_ERROR);
      client_disconnects(wifiServer);
      break;

      case 5:
      EXPECT_EQ(state, CLIENT_CONNECTED);
      EXPECT_EQ(cond.error, STATE_CHECK_FAILED);
      wifi_disconnects(wifiServer);
      break;

      case 6:
      case 7:
      case 8:
      case 9:
      // will not allow that connect succeeds
      wifi_disconnects(wifiServer);
      EXPECT_EQ(cond.error, STATE_CHECK_FAILED);
      break;

      // needs 3 iterations back to disconnected
      case 10:
      case 11:
      EXPECT_EQ(state, ERR_SSID_NOT_AVAIL);
      EXPECT_EQ(cond.error, STATE_CHECK_FAILED);
      WiFi.setNumSSIDs(0);
      break;

      case 12:
      WiFi.setNumSSIDs(1);
      break;

      case 13:
      EXPECT_EQ(state, DISCONNECTED);
      EXPECT_EQ(cond.error, NO_ERROR);
      break;

      case 14:
      EXPECT_EQ(state, CONNECTED);
      EXPECT_EQ(cond.error, NO_ERROR);
      break;

    } 
  }
}

TEST(StaticHandler, runthruWithConnectDisconnect){
  
    RobustWiFiServer wifiServer;
  
    wifiServer.init(myIP, gateway, subnet, port, ssid, password);
    wifiServer.connect();
    prepareState(DISCONNECTED, wifiServer);
    EXPECT_EQ( wifiServer.getState(), DISCONNECTED);
  
    // protocol:
    // connects ordinary
    // client disconnects
    // wifi breaks down
    // wifi back
    // connect ordinary
    for (int i=0; i<=10; i++){
      printf("%d\n", i);
  
      wifiServer.loop();
      ServerCondition cond = wifiServer.getCondition();
      
      ServerState state = wifiServer.getState();
      switch(i){
        case 0:
        EXPECT_EQ(state, DISCONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        //prepareState(CONNECTED, wifiServer);
        break;
      
        case 1:
        EXPECT_EQ(state, CONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        //prepareState(SERVER_LISTENING, wifiServer);
        break;
  
        case 2:
        EXPECT_EQ(state, SERVER_LISTENING);
        EXPECT_EQ(cond.error, NO_ERROR);
        client_connect(wifiServer);
        break;
  
        case 3:
        EXPECT_EQ(state, CLIENT_CONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        client_send_data(wifiServer);
        break;

        case 4:
        EXPECT_EQ(state, DATA_AVAILABLE);
        EXPECT_EQ(cond.error, NO_ERROR);
        wifiServer.disconnect();
        break;
  
        case 5:
        EXPECT_EQ(state, DATA_AVAILABLE);
        EXPECT_EQ(cond.error, NO_ERROR);
        break;
  
        case 6:
        EXPECT_EQ(state, CLIENT_CONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        break;
  
        case 7:
        EXPECT_EQ(state, SERVER_LISTENING);
        EXPECT_EQ(cond.error, NO_ERROR);
        break;

        case 8:
        EXPECT_EQ(state, CONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        break;

        case 9:
        EXPECT_EQ(state,DISCONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        break;

        case 10:
        EXPECT_EQ(state,DISCONNECTED);
        EXPECT_EQ(cond.error, NO_ERROR);
        break;


      }
    }
}
  

TEST(StaticHandler, runWithTimeout){
  
    RobustWiFiServer wifiServer;
  
    wifiServer.init(myIP, gateway, subnet, port, ssid, password);
    wifiServer.connect();
    prepareState(DISCONNECTED, wifiServer);
    EXPECT_EQ( wifiServer.getState(), DISCONNECTED);
    WiFi.setNumSSIDs(0);


    for (int i=0; i<=7; i++){
      printf("%d\n", i);
      mock_increaseTime(1000);
  
      wifiServer.loop();
      // dont succeed server start
      prepareState(DISCONNECTED, wifiServer);

      ServerCondition cond = wifiServer.getCondition();

      switch(i){
        case 0:
        case 1:
        case 2:
        case 3:
        EXPECT_EQ(cond.error, NO_ERROR);
        EXPECT_EQ(cond.numberOfTimeouts, 0);
        break;

        case 4:
        // timeout occured
        EXPECT_EQ(cond.error, TRANSITION_TIMEOUT_REACHED);
        EXPECT_EQ(cond.numberOfTimeouts, 1);
        break;

      }
    }
    
}  