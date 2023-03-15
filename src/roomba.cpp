#include "roomba.h"

#include <ESPTelnet.h>
// https://github.com/johnboiles/esp-roomba-mqtt/blob/master/lib/Roomba/Roomba.cpp
Roomba::Roomba()
{
	periodics_ = {
		{ voltage_, 10000, 22},
		{ current_, 1000, 23},
		{ dirt_, 100, 15},
	};
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
	if (Serial.getRxBufferSize() == 0) return;
	
	if (Serial.available())
	{
		uint8_t c=(uint8_t)(Serial.read());

		received++;
		if (readBusy())
		{
			*read_ptr_ = c;
			if (read_left_ > 0)
			{
				++read_ptr_;
				read_left_--;
			}
			else
			{
				--read_ptr_;
				read_left_++;
			}
			if (read_left_ == 0 and read_cb_)
			{
				read_cb_(read_buf_);
			}
		}
		else
		{
			if (unexpected_read_.length() > 20)
				unexpected_read_.erase(0,1);
			unexpected_read_ += (char)c;
			unexpected_bytes_++;
		}
		return;
	}
	if (readBusy() and millis() > read_timeout_)
	{
		stopRead();
	}
	if (not readBusy())
	{
		auto now = millis();
		for(auto &periodic: periodics_)
		{
			bool read=false;

			while (now >= periodic.next)
			{
				read = true;
				periodic.next += periodic.ms;
			}
			if (read)
			{
				Serial.write(142);
				Serial.write(periodic.packetId);
				uint8_t* dest = periodic.buf;
				int8_t len = periodic.len;
				readBytes(len,
					[dest, len](uint8_t *buf)
					{
						memcpy(dest, buf, std::abs(len));
					});
				break;
			}
		}
	}
}