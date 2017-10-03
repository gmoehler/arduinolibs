/*
    This sketch shows the Ethernet event usage
*/

#include "WifiHandler.h"

char* ssid     = "MY_SSID";
char* password = "my_password";

IPAddress myIP(192,168,1,127);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

uint16_t port = 1110;

void setup()
{
    Serial.begin(115200);
    WifiHandler::init(myIP, gateway, subnet, port, ssid, password);
    WifiHandler::setTargetState(DATA_AVAILABLE);
}

void loop()
{
    WifiHandler::loop();
    if (WifiHandler::getState() == DATA_AVAILABLE) {
        char c = WifiHandler::readData();           // read a byte, then
        Serial.print(static_cast<uint8_t> (c));     // print it out the serial monitor
        Serial.print(" ");                   
    }
}