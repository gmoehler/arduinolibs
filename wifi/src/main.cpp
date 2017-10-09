/*
    This sketch shows the Ethernet event usage
*/

#include "RobustWiFiServer.h"

char* ssid     = "MY_SSID";
char* password = "my_password";

IPAddress myIP(192,168,1,127);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

uint16_t port = 1110;

RobustWiFiServer wifiServer;

void setup()
{
    Serial.begin(115200);
    wifiServer.init(myIP, gateway, subnet, port, ssid, password);
    wifiServer.setTargetState(DATA_AVAILABLE);
}

void loop()
{
    wifiServer.loop();
    if (wifiServer.getState() == DATA_AVAILABLE) {
        char c = wifiServer.readData();           // read a byte, then
        Serial.print(static_cast<uint8_t> (c));     // print it out the serial monitor
        Serial.print(" ");                   
    }
}