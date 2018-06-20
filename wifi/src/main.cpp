/*
    This sketch shows the Ethernet event usage
*/

#include "RobustWiFiServer.h"
#include "wifi_utils.h"

const String ssid     = "MY_SSID7";
const String password = "my_password";

IPAddress myIP(192,168,1,127);
IPAddress myIP2(192,168,1,130);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

uint16_t port = 1110;

RobustWiFiServer wifiServer;

bool connecting = true;
bool dataRead = false;
bool ip0 = true;

void setup()
{
    Serial.begin(115200);
    wifiServer.init(myIP, gateway, subnet, port, ssid, password);
    wifiServer.connect();
}

void loop()
{
    wifiServer.loop();

/*    if (wifiServer.getState() == DATA_AVAILABLE) {
        char c = wifiServer.readData();           // read a byte, then
        Serial.print(static_cast<uint8_t> (c));   // print it out the serial monitor
        Serial.print(" ");   
        dataRead = true;                
    }
    else */
    if (connecting && wifiServer.getState() == SERVER_LISTENING){
        delay(5000);
        Serial.println("\n*** Now disconnecting... ***\n");
        wifiServer.disconnect();
        connecting = false;
        dataRead = false;
    }
    else if (!connecting && wifiServer.getState() == DISCONNECTED) {
        delay(5000);
        Serial.println("\n*** Now connecting... ***\n");
        IPAddress ip = ip0 ? myIP : myIP2;
        ip0 = !ip0;
        wifiServer.connect(ip);
        connecting = true;
    }

}