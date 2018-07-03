/*
    This sketch shows the Ethernet event usage
*/

#include <WiFi.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_wps.h"
#include "esp_event_loop.h"

const char* ssid     = "MY_SSID";
const char* password = "my_password";

IPAddress myIP(192,168,1,127);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
uint16_t port = 1110;

WiFiServer server(port);


void serverStartup(){
  server.begin();
  server.setTimeout(5);
  delay(1000);
  Serial.println("Waiting for client...");

  WiFiClient client = server.available();

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    while (client.connected()) {            // loop while the client's connected
      client.setTimeout(5);                  // timeout does not seem to work
    //Serial.print("Client connected");
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        printf("%d",c);                    // print it out the serial monitor
      }
    }

    client.stop();
    Serial.println("Client Disconnected.");
  }
  Serial.println("Ending server.");
  server.end();
}

void setup()
{
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    delay(100);

    WiFi.config(myIP,gateway,subnet);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop()
{
    delay(1000);
    serverStartup();
}