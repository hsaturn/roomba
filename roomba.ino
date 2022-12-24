#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPTelnet.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <TinyConsole.h>
#include <ArduinoOTA.h>

#include "roomba.h"
#include "logo.h"
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

ESPTelnet &operator <<(ESPTelnet &obj, std::string arg) { obj.print(arg.c_str()); return obj; } 
std::string topic="sensor/temperature";

MqttBroker broker(1883);
MqttClient mqtt(&broker);

ESPTelnet telnet;

void onPublish(const MqttClient* /* source */, const Topic& topic, const char* payload, size_t /* length */)
{ Serial << "--> client A received " << topic.c_str() << ", " << payload << endl; }

void onInputReceived(String str)
{
	static std::string cmd;
	if (str=="\r" or str=="\n")
	{
		if (cmd.length())
		{
			Serial << "telnet: " << cmd << endl;
			telnet << "command: " << cmd << endl;
			std::string first = Command::firstWord(cmd);
			if (first == "help")
			{
				Command::help(cmd, telnet);
			}
			else if (not Command::handle(cmd))
			{
				telnet << "Unknow command: " << cmd << endl;
			}
			telnet.print("> ");
			cmd.clear();
		}
	}
	else
		cmd += std::string(str.c_str());
}

void onConnect(String ip)
{
  Serial << "Incoming connection " << ip << endl;
  telnet << "Busybox telnet\n\r> ";
}

void setupTelnet()
{
  telnet.setLineMode(false);
  telnet.onInputReceived(onInputReceived);
  telnet.onConnect(onConnect);
  telnet.begin(23);
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

void setupLogo()
{
	Command::addHandler(new Logo());
}

void setup()
{
  Serial.begin(115200); // TODO move to roomba class
  setupWifi();
  setupOta();
  setupMqtt();
  setupTelnet();
  setupLogo();
}

void loop()
{
  ArduinoOTA.handle();
  telnet.loop();
  mqtt.loop();
}
