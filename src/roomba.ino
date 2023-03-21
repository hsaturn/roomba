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

std::string topic="sensor/temperature";

MqttBroker broker(1883);
MqttClient mqtt(&broker);

ESPTelnet telnet;
Roomba roomba;

Flash flash(5000);
Lidar* lidar = nullptr;
Nanos* nanos = nullptr;

void onInputReceived(String str)
{
  #if 0
  telnet << '[';
  for(const auto& c: str)
  {
    if (c < ' ' or c>127)
      telnet << '<' << (int)c << '>';
    else
      telnet << c;
  }
  telnet << ']' << endl;
  #endif

  flash.flash();
	static std::string cmd;
  if (str=="\r")
  {

  }
	else if (str=="\n")
	{
		if (cmd.length())
		{
			std::string first = Command::firstWord(cmd);
      Command::Params p(roomba, cmd, telnet);
			if (Command::handle(p))
			{
				telnet.print("ok");
			}
			else
			{
				telnet << "Unknow command: " << first;
			}
    }
    telnet << endl;
    telnet.print(hostname.c_str());
    telnet.print(" > ");
    cmd.clear();
  }
	else
		cmd += std::string(str.c_str());
}

void onPublish(const MqttClient* /* source */, const Topic& topic, const char* payload, size_t /* length */)
{
  std::string t = topic.str();
  std::string device = Command::getWord(t, '/');
  if (device == "roomba")
  {
    if (t == "exec")
    {
      telnet << "mqtt: received " << topic.c_str() << ", " << payload << endl;
      onInputReceived(payload);
    }
    else
    {
      onInputReceived(t.c_str());
    }
    onInputReceived("\n");
  }
}

void onConnect(String ip)
{
  telnet << "Roomba v" << AUTO_VERSION << endl;
  onInputReceived("\n");
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
  nanos = new Nanos;
  Command::addHandler(nanos);
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
  roomba.loop(&mqtt, &telnet);
  Command::loops();

  static unsigned long last = 0;
  std::string un = roomba.unexpectedChars();
  if (un.size())
  {
    if (millis() - last > 100)
      telnet << endl;
    last = millis();
    while (un.size())
    {
      char c=un[0];
      if (c >= 32 or c==13 or c==10 or c==9)
        telnet << c;
      else
        telnet << '<' << (int)c << '>';
      un.erase(0,1);
    }
  }

  if (nanos)
  {
    std::string cmd = nanos->getEvery();
    if (cmd.length())
    {
      Command::Params p(roomba, cmd, telnet);
      Command::handle(p);
    }
  }
}
