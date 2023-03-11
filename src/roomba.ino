#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPTelnet.h>
#include <TinyMqtt.h>   // https://github.com/hsaturn/TinyMqtt
#include <TinyConsole.h>
#include <ArduinoOTA.h>

#include "roomba.h"
#include "roomba_command.h"
#include "nanos.h"
#include "logo.h"
#include "lidar.h"
#include "auth.h"
#include "flash.h"

const std::string hostname="Roomba";

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
Roomba roomba;

Flash flash(5000);
Lidar* lidar = nullptr;

void onInputReceived(String str)
{
  flash.flash();
	static std::string cmd;
	if (str=="\r" or str=="\n")
	{
		if (cmd.length())
		{
			std::string first = Command::firstWord(cmd);
      Command::Params p(roomba, cmd, telnet);
			if (Command::handle(p))
			{
				telnet.println("ok");
			}
			else
			{
				telnet << "Unknow command: " << first << endl;
			}
			telnet.print(hostname.c_str());
			telnet.print(" > ");
			cmd.clear();
		}
	}
	else
		cmd += std::string(str.c_str());
}

void onPublish(const MqttClient* /* source */, const Topic& topic, const char* payload, size_t /* length */)
{
  if (topic.str() == "roomba/exec")
  {
    telnet << "mqtt: received " << topic.c_str() << ", " << payload << endl;
    onInputReceived(payload);
  }
}

void onConnect(String ip)
{
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
  WiFi.persistent(false);
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
  ArduinoOTA.setHostname(hostname.c_str());
  ArduinoOTA.setPasswordHash("f64e51d7d8de34ef350a526467e0a610"); // ..5..

  ArduinoOTA.onStart([]() {});
  ArduinoOTA.onEnd([]() {});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { });
  ArduinoOTA.onError([](ota_error_t error) {});

  ArduinoOTA.begin();
}

void setupMqtt()
{
  broker.begin();
  mqtt.setCallback(onPublish);
  mqtt.subscribe("#");
}

void setupRoomba()
{
	Command::addHandler(new RoombaCommand());
}

void setupLogo()
{
	Command::addHandler(new Logo());
}

void setupNanos()
{
  Command::addHandler(new Nanos());
}

void setupLidar()
{
  lidar = new Lidar(telnet, &mqtt);
  Command::addHandler(lidar);
}

void setup()
{
  setupWifi();
  setupOta();
  setupTelnet();
  setupMqtt();
  setupNanos();
  setupRoomba();
  setupLogo();
  setupLidar();
}

void loop()
{
  ArduinoOTA.handle();
  telnet.loop();
  broker.loop();
  mqtt.loop();
  flash.loop();
  Command::loops();
}
