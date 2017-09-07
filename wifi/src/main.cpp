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

const char* ssid     = "My_SSID";
const char* password = "mypassword";

WiFiServer server(80);

// https://github.com/espressif/arduino-esp32/blob/ca7f6cc516ce842cf7c6b35dbde4b0d4a5a86404/tools/sdk/include/esp32/esp_event.h

void WiFiEvent(WiFiEvent_t event)
{
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

void smartConnect(){
  WiFi.beginSmartConfig();
  
  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void startWps(){
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  
  Serial.println("start wps...");
  ESP_ERROR_CHECK(esp_wifi_wps_enable(WPS_TEST_MODE));
  ESP_ERROR_CHECK(esp_wifi_wps_start(0));

}

void serverStartup(){
  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      Serial.println("Client connected");
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          Serial.println("HIGH");
          //digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          Serial.println("LOW");
          //digitalWrite(5, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void initWifiConnect(){

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.println("Waiting for WiFi... ");  
}

void setup()
{
    Serial.begin(115200);

    //WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.onEvent(WiFiEvent);

    delay(100);

    scanNetworks();
    initWifiConnect();
   //smartConnect();
   //startWps();
}


void loop()
{
    delay(1000);
    if (wifi_connected && !done) {
      //testClient("http://vvs.de", 80);
      serverStartup();
      done = true;
    }

}