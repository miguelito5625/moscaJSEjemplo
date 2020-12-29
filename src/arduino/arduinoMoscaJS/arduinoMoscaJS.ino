#include <ESP8266WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <EEPROM.h>


void callback(char *topic, byte *payload, unsigned int length1);

//const char *mqtt_server = "broker.emqx.io";
//char mqtt_server[40] = "broker.emqx.io";
char mqtt_server[40] = "XXXX";
char mqtt_topic[80] = "XXXX";
char mqtt_client_id[30] = "";


//flag for saving data
bool shouldSaveConfig = false;

WiFiClient espclient;
PubSubClient client(mqtt_server, 1883, callback, espclient);



//#define LED_PIN 0
#define OTHER_PIN 0

int estadoPin = 0;

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
  shouldSaveConfig = true;
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
  EEPROM.begin(512);


  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // put your setup code here, to run once:
  //  Serial.begin(115200);

  //set led pin as output
  pinMode(LED, OUTPUT);
  pinMode(OTHER_PIN, OUTPUT);
  digitalWrite(OTHER_PIN, estadoPin);

  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  // id/name, placeholder/prompt, default, length
  WiFiManagerParameter custom_mqtt_server("server", "Mqtt Server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_topic("topic", "Mqtt topic", mqtt_server, 80);


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //reset settings - for testing
  // wm.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_topic);

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

  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_topic, custom_mqtt_topic.getValue());

  Serial.println("EL MQTT SERVER: " + (String)mqtt_server);


  if (shouldSaveConfig) {
    Serial.println("SE CAMBIO MQTT SERVER: " + (String)mqtt_server);
    Serial.println("SE CAMBIO MQTT TOPIC: " + (String)mqtt_topic);
    writeString(350, (String)mqtt_server);
    writeString(400, (String)mqtt_topic);
    EEPROM.commit();
  } else {
    strcpy(mqtt_server, read_String(350).c_str());
    strcpy(mqtt_topic, read_String(400).c_str());
    //    Serial.println("prueba mqtt server en eeprom: " + read_String(350));
    //    Serial.println("prueba mqtt topic en eeprom: " + read_String(400));

  }

  Serial.println("EL MQTT SERVER: " + (String)mqtt_server);
  Serial.println("EL MQTT TOPIC: " + (String)mqtt_topic);

  String generateClientId = "Esp8266Client" + generarId(10);
  strcpy(mqtt_client_id, generateClientId.c_str());
  Serial.println("EL MQTT CLIENT ID: " + (String)mqtt_client_id);

  ticker.detach();
  //keep LED on
  digitalWrite(LED, LOW);

  reconnect();
}

void callback(char *topic, byte *payload, unsigned int length1)
{
  Serial.print("Mensaje del topic: [");
  Serial.print(topic);
  Serial.println("]");
  String myString = "";
  for (int i = 0; i < length1; i++)
  {
    //        Serial.print(payload[i]);
    myString += (char)payload[i];
  }

  Serial.println("Datos recibidos: " + myString);
  estadoPin = myString.toInt();

}

void reconnect()
{
  Serial.println("Mqtt server en reconnect(): " + (String)mqtt_server);

  //    while (WiFi.status() != WL_CONNECTED)
  //    {
  //        delay(500);
  //        Serial.print(".");
  //    }
  while (!client.connected())
  {
    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado al MQTT server: " + (String)mqtt_server);

      client.subscribe(mqtt_topic);
      //      client.subscribe("mike/5625/luzsalacasamama");
      //            client.subscribe("testTopic");
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
  controlEncendido();
  modoFiesta();
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();
}



void controlEncendido() {
  if (estadoPin < 2) {
    digitalWrite(OTHER_PIN, estadoPin);
  }
}

void modoFiesta() {
  if (estadoPin == 2) {
    client.publish("mike/5625/recibido", "modo fiesta");
    digitalWrite(OTHER_PIN, 1);
    delay(random(100, 800));
    digitalWrite(OTHER_PIN, 0);
    delay(random(100, 800));
  }
}

void writeString(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  //  EEPROM.commit();
}


String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}


String generarId(int tamanio) {
  String id = "";
  for (int i = 0; i <= tamanio; i++) {
    id += (String)random(0, 9);
  }
  return id;
}
