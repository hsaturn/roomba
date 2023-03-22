#include "roomba.h"

#include <ESPTelnet.h>
// https://github.com/johnboiles/esp-roomba-mqtt/blob/master/lib/Roomba/Roomba.cpp
Roomba::Roomba()
{
	periodics_ = {
		{ voltage_, 10000, 22, "batt/voltage"},
		{ current_, 1000, 23 , "batt/current" },
		{ capacity_, 10000, 26, "batt/capacity" },
		{ charge_, 2000, 21, "batt/state" },

		{ dirt_, 100, 15, "sensor/dirt" },
		{ buttons_, 20, 18, "sensor/buttons" },
		{ temp_, 1000, 24, "sensor/temp" },
		{ wall_, 50, 27, "sensor/wall" },
		{ lclift_, 50, 28, "sensor/clifts/left" },
		{ flclift_, 50, 29, "sensor/clifts/front_left" },
		{ frclift_, 50, 30, "sensor/clifts/front_right" },
		{ rclift_, 50, 31, "sensor/clifts/right" },
		{ bumpers_, 20, 45, "sensor/bump/all"},
		{ bump_l_, 50, 46, "sensor/bump/left" },
		{ bump_fl_, 50, 47, "sensor/bump/front_left" },
		{ bump_cl_, 50, 48, "sensor/bump/center_left" },
		{ bump_cr_, 50, 49, "sensor/bump/center_right" },
		{ bump_fr_, 50, 50, "sensor/bump/front_right" },
		{ bump_r_, 50, 51, "sensor/bump/right" },
		{ stasis_, 50, 58, "sensor/statis" },

		{ velocity_, 20, 39, "velocity/velocity" },
		{ radius_, 20, 40, "velocity/radius" },
		{ rvelocity_, 20, 41, "velocity/right" },
		{ lvelocity_, 20, 42, "velocity/left" },
		{ lsteps_, 20, 43, "steps/left" },
		{ rsteps_, 20, 44, "steps/right" },

	};
	looper_ = periodics_.begin();
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

void Roomba::loop(MqttClient* mqtt, OutputStream* out)
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
		auto ms = millis();
		if (looper_ == periodics_.end())
		{
			last_ploop_ = ms-ms_begin_;
			if (ms_begin_ and mqtt) mqtt->publish("last_ploop", String(last_ploop_));
			ms_begin_ = ms;
			looper_ = periodics_.begin();
		}
		else
		{
			auto& periodic = *looper_;
			bool read=false;

			while (periodic.ms and ms >= periodic.next)
			{
				read = true;
				periodic.next += periodic.ms;
			}
			if (read)
			{
				Serial.write(142);
				Serial.write(periodic.packetId);
				readBytes(periodic.len,
					[this, out, &periodic, mqtt](uint8_t *buf)
					{
						if (periodic.topic)
						{
							sent_++;
							if (memcmp(periodic.buf, buf, std::abs(periodic.len)))
							{
								String s;
								if (periodic.len==2)
									s = String(*reinterpret_cast<uint16_t*>(buf));
								else if (periodic.len==-2)
									s = String(*reinterpret_cast<int16_t*>(buf));
								else if (periodic.len==1)
									s = String(*reinterpret_cast<uint8_t*>(buf));

								if (s.length())
								{
									mqtt->publish(periodic.topic, s, true);
								}
							}
						}
						memcpy(periodic.buf, buf, std::abs(periodic.len));
					});
			}
			++looper_;
		}
	}
}