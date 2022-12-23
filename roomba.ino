#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <TinyConsole.h>
#include "Roomba.h"

/** 
  * 
  *            +------------------------------+
  *            | ESP                          |
  *            |                  +--------+  | 1883 <--- Wifi mqtt client/s
  *            |                  | broker |  |
  *            |                  +--------+  |
  *            |                      ^       |
  *            |                      |       |
  *            |                      |       |
  * Hardware   |                      v       |
  * Serial     |  +--------+   +----------+   |
  *    <-------+ >| Roomba |<->| Mqtt     |   |
  * Roomba     |  +--------+   +----------+   |
  *            |        ^                     |
  *            |         \     +----------+   |
  *            |          `--->| Console  |<--+------------>  USB Serial
  *            |               +----------+   |
  *            |                              |
  *            +------------------------------+
  * 
  */

const char *ssid     = "Freebox-786A2F";
const char *password = "usurpavi8dalum64lumine?";

std::string topic="sensor/temperature";

MqttBroker broker(1883);

MqttClient mqtt(&broker);

void onPublish(const MqttClient* /* source */, const Topic& topic, const char* payload, size_t /* length */)
{ Serial << "--> client A received " << topic.c_str() << ", " << payload << endl; }

void setup()
{
  Serial.begin(115200);   // Usb serial
  Serial1.begin(115200);  // Hardware serial

  

  delay(100);
  Serial << "Clients with wifi " << endl;

	if (strlen(ssid)==0)
		Serial << "****** PLEASE EDIT THE EXAMPLE AND MODIFY ssid/password *************" << endl;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {   Serial << '-'; delay(500); }

  Serial << "Connected to " << ssid << "IP address: " << WiFi.localIP() << endl;

  broker.begin();

  mqtt.setCallback(onPublish);
  mqtt.subscribe("#");
}

void loop()
{
  broker.loop();
  mqtt.loop();

}
