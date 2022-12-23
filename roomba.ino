#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPTelnet.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <TinyConsole.h>
#include <ArduinoOTA.h>

#include "roomba.h"
#include "auth.h"

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
std::string topic="sensor/temperature";

MqttBroker broker(1883);
MqttClient mqtt(&broker);

ESPTelnet telnet;

void onPublish(const MqttClient* /* source */, const Topic& topic, const char* payload, size_t /* length */)
{ Serial << "--> client A received " << topic.c_str() << ", " << payload << endl; }

void onCommand(String str)
{
  Serial << "telnet: " << str << "\n";
  telnet << "command: " << str << endl;
}

void onConnect(String ip)
{
  Serial << "Incoming connection " << ip << endl;
  telnet << "Welcome" << endl;
}

void setupTelnet()
{
  telnet.setLineMode(true);
  telnet.begin(23);
  telnet.onInputReceived(onCommand);
  telnet.onConnect(onConnect);
}

void setupWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
}

void setupOta()
{
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("RoombaTest");
  ArduinoOTA.setPasswordHash("f64e51d7d8de34ef350a526467e0a610"); // ..5..

  ArduinoOTA.onStart([]() {});
  ArduinoOTA.onEnd([]() {});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { });
  ArduinoOTA.onError([](ota_error_t error) {});

  ArduinoOTA.begin();
}

void setupMqtt()
{
  mqtt.setCallback(onPublish);
  mqtt.subscribe("#");
}

void setup()
{
  Serial.begin(115200); // TODO move to roomba class
  setupWifi();
  setupOta();
  setupMqtt();
  setupTelnet();
}

void loop()
{
  ArduinoOTA.handle();
  telnet.loop();
  mqtt.loop();
}
