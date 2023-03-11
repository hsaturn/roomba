#pragma once

#include <TinyMqtt.h>
#include "Command.h"
#include <Wire.h>
#include <VL53L1X.h>	// Pololu

class Lidar : public Command
{
	public:
		Lidar(OutputStream&, MqttClient*);

		const char* name() const override { return "lidar"; }

		void loop();
		void init();

		bool ok() const { return last_update != 0; }
		unsigned long lastUpdate() const { return last_update; }
		uint16_t mm() const { return distance; }

	private:
		VL53L1X sensor;
		unsigned long last_update = 0;	// 0 : bad_init
		uint16_t distance;
		MqttClient* mqtt;
};
