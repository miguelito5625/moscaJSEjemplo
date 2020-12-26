#include <ESP8266WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>


void callback(char *topic, byte *payload, unsigned int length1);

const char *mqtt_server = "192.168.1.100";
WiFiClient espclient;
PubSubClient client(mqtt_server, 1883, callback, espclient);

//#define LED_PIN 0
#define OTHER_PIN 0

//for LED status
#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;

void tick()
{
    //toggle state
    digitalWrite(LED, !digitalRead(LED)); // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    //if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
    //entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
}

void setup()
{

    Serial.begin(115200);

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // put your setup code here, to run once:
    Serial.begin(115200);

    //set led pin as output
    pinMode(LED, OUTPUT);
    pinMode(OTHER_PIN, OUTPUT);
    // start ticker with 0.5 because we start in AP mode and try to connect
    ticker.attach(0.6, tick);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    //reset settings - for testing
    // wm.resetSettings();

    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wm.setAPCallback(configModeCallback);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if (!wm.autoConnect())
    {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(1000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    ticker.detach();
    //keep LED on
    digitalWrite(LED, LOW);

    

    reconnect();
}

void callback(char *topic, byte *payload, unsigned int length1)
{
    Serial.print("message arrived[");
    Serial.print(topic);
    Serial.println("]");

    for (int i = 0; i < length1; i++)
    {
        Serial.print(payload[i]);
    }
    if (payload[0] == 49)
        digitalWrite(LED, HIGH); //ASCII VALUE OF '1' IS 49
    else if (payload[0] == 50)
        digitalWrite(LED, LOW); //ASCII VALUE OF '2' IS 50
    Serial.println();
}

void reconnect()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    while (!client.connected())
    {
        if (client.connect("ESP8266Client123456789"))
        {
            Serial.println("connected");
            client.subscribe("ledcontrol");
        }
        else
        {
            Serial.print("failed,rc=");
            Serial.println(client.state());
            delay(500);
        }
    }
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }

    client.loop();
}
