#pragma once

#include "module.h"

#include <TinyMqtt.h>
#include <Wire.h>
#include <VL53L1X.h>	// Pololu

// Note see there https://community.st.com/s/question/0D53W000005pkkPSAQ/vl53l1x-gpio1-interrupt

class Lidar : public Module
{
	public:
		Lidar(OutputStream&, MqttClient*);

		const char* name() const override { return "lidar"; }

		void loop();

		void init();
		void stop();

		bool ok() const { return last_update != 0; }
		unsigned long lastUpdate() const { return last_update; }
		uint16_t mm() const { return distance; }

	private:
		VL53L1X sensor;
		unsigned long last_update = 0;	// 0 : bad_init
		uint16_t distance;
		MqttClient* mqtt;
};
