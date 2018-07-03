/*
    This sketch shows the Ethernet event usage
*/

#include <WiFi.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_wps.h"
#include "esp_event_loop.h"


#ifndef PIN2STR
#define PIN2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7]
#define PINSTR "%c%c%c%c%c%c%c%c"
#endif

//#define WPS_TEST_MODE WPS_TYPE_PBC
#define WPS_TEST_MODE WPS_TYPE_PIN
//#define WPS_TEST_MODE WPS_TYPE_DISABLE

static bool wifi_connected = false;
static bool done=false;

const char* ssid     = "MY_SSID";
const char* password = "my_password";

IPAddress myIP(192,168,1,127);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
uint16_t port = 1110;

WiFiServer server = WifiServer2(port);

// https://github.com/espressif/arduino-esp32/blob/ca7f6cc516ce842cf7c6b35dbde4b0d4a5a86404/tools/sdk/include/esp32/esp_event.h

const char * system_event_reasons[] = { "UNSPECIFIED", "AUTH_EXPIRE", "AUTH_LEAVE", "ASSOC_EXPIRE", "ASSOC_TOOMANY", "NOT_AUTHED", "NOT_ASSOCED", "ASSOC_LEAVE", "ASSOC_NOT_AUTHED", "DISASSOC_PWRCAP_BAD", "DISASSOC_SUPCHAN_BAD", "IE_INVALID", "MIC_FAILURE", "4WAY_HANDSHAKE_TIMEOUT", "GROUP_KEY_UPDATE_TIMEOUT", "IE_IN_4WAY_DIFFERS", "GROUP_CIPHER_INVALID", "PAIRWISE_CIPHER_INVALID", "AKMP_INVALID", "UNSUPP_RSN_IE_VERSION", "INVALID_RSN_IE_CAP", "802_1X_AUTH_FAILED", "CIPHER_SUITE_REJECTED", "BEACON_TIMEOUT", "NO_AP_FOUND", "AUTH_FAIL", "ASSOC_FAIL", "HANDSHAKE_TIMEOUT" };
#define reason2str(r) ((r>176)?system_event_reasons[r-176]:system_event_reasons[r-1])

void WiFiEvent1(WiFiEvent_t event, system_event_info_t event_info)
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
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_STOP:
      printf("Wifi disconnected.\n");
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      printf("Wifi connected.\n");
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      printf("Authmode of Access Point has changed.\n");
      wifi_connected = false;
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      printf("IP address: %s\n", WiFi.localIP().toString().c_str());
      wifi_connected=true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("WiFi lost connection.\n");
      reason = event_info.disconnected.reason;
      printf("Reason: %u - %s\n", reason, reason2str(reason));
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      printf("WiFi connected by WPS.\n");
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      printf("WPS connection failed.\n");
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      printf("WPS connection timeout.\n");
      wifi_connected=false;
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      printf("WPS with pin mode: %s\n"/*, PIN2STR(event.event_info.sta_er_pin.pin_code)*/);
      wifi_connected=false;
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

void scanNetworks(){
  Serial.println("scan start");
  
      // WiFi.scanNetworks will return the number of networks found
      int n = WiFi.scanNetworks();
      Serial.println("scan done");
      if (n == 0) {
          Serial.println("no networks found");
      } else {
          Serial.print(n);
          Serial.println(" networks found");
          for (int i = 0; i < n; ++i) {
              // Print SSID and RSSI for each network found
              Serial.print(i + 1);
              Serial.print(": ");
              Serial.print(WiFi.SSID(i));
              Serial.print(" (");
              Serial.print(WiFi.RSSI(i));
              Serial.print(")");
              //https://github.com/espressif/arduino-esp32/blob/ca7f6cc516ce842cf7c6b35dbde4b0d4a5a86404/tools/sdk/include/esp32/esp_wifi_types.h
              Serial.println(WiFi.encryptionType(i));
              Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
              delay(10);
          }
      }
  Serial.println("");
}

void testClient(const char * host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

void serverStartup(){
  
  Serial.println("Starting server\n");
  server.begin();
  delay(100);

  Serial.println("Setting server timeout\n");
  //int st1 = server.setTimeout(5);
  //if (st1 != 0)
  //  printf("timeout error %d ", st1);
  delay(1000);

  Serial.println("Waiting for client...");

  WiFiClient client;   // listen for incoming clients
  //client.setTimeout(10);             // timeout does not seem to work
  client = server.available();

  if (client) {                             // if client is connected
    Serial.println("New Client.");           // print a message out the serial port
    //String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      int st = client.setTimeout(2);             // timeout does not seem to work
      if (st != 0)
         printf("%d ", st);
      //Serial.print(".");
    //Serial.print("Client connected");
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        printf("%d",c);                    // print it out the serial monitor
      }
    }
    // close the connection:
    Serial.println("Closing connection\n");
    client.stop();
    delay(100);

    Serial.println("Client Disconnected.");
  }
  Serial.println("Ending server.");
  server.end();
}

void initWifiConnect(){

  Serial.println("Configuring WiFi... ");  
  WiFi.config(myIP,gateway,subnet);
  delay(100);

  Serial.println("Starting WiFi... ");  
  WiFi.begin(ssid, password);
  delay(100);
  Serial.println("Waiting for WiFi... ");
  delay(100);  
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Listening to WiFi envents... ");  
    WiFi.onEvent(WiFiEvent1);
    delay(100);
    
    Serial.println("Setting STA mode... ");  
    WiFi.mode(WIFI_STA);
    delay(100);

    /* Serial.println("Disconnecting... ");  
    WiFi.disconnect();
    delay(100);
*/
    //scanNetworks();
    Serial.println("Init WiFi  connect... ");  
    initWifiConnect();
    delay(100);
}


void loop()
{
    delay(1000);
    if (wifi_connected) {
      //testClient("http://vvs.de", 80);
      serverStartup();
    }

}