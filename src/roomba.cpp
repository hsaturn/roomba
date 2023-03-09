#include "roomba.h"

// https://github.com/johnboiles/esp-roomba-mqtt/blob/master/lib/Roomba/Roomba.cpp
Roomba::Roomba()
{
}

void Roomba::start()
{
	Serial.end();
	delay(100);
	Serial.begin(115200);
	delay(100);
	pinMode(BRC_PIN, OUTPUT);
	digitalWrite(BRC_PIN, LOW);
	delay(200);
	digitalWrite(BRC_PIN, INPUT);
	delay(200);
	passive();
}

void Roomba::motors()
{
	Serial.write(144);
	Serial.write(main_brush_);
	Serial.write(side_brush_);
	Serial.write(vacuum_);
}

bool Roomba::driveDirect(int16_t leftVelocity, int16_t rightVelocity)
{
	Serial.write(145);
	Serial.write((rightVelocity & 0xff00) >> 8);
	Serial.write(rightVelocity & 0xff);
	Serial.write((leftVelocity & 0xff00) >> 8);
	Serial.write(leftVelocity & 0xff);
	return true;
}

bool Roomba::drive(int16_t velocity, int16_t radius)
{
  Serial.write(137);
  Serial.write((velocity & 0xff00) >> 8);
  Serial.write(velocity & 0xff);
  Serial.write((radius & 0xff00) >> 8);
  Serial.write(radius & 0xff);
	return true;
}

void Roomba::loop()
{
}
