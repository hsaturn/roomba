#pragma once
#include <string>
#include <Arduino.h>

#define BRC_PIN 14

class Roomba
{
  public:
    Roomba();

    void loop();

    void start();
    void off()      { Serial.write(133); }
    void clean()    { Serial.write(135); }
    void max()      { Serial.write(136); }
    void spot()		  { Serial.write(134); }
    void seekDock()	{ Serial.write(143); }
    void reset()	  { Serial.write(7);   }
    void stop()		  { Serial.write(173); }
    void dock()			{ Serial.write(143); }
    void raw(const std::string& buffer) { Serial.write(buffer.c_str(), buffer.length()); }

    void passive()  { Serial.write(128); }
    void safe()     { Serial.write(131); }
    void full()     { Serial.write(132); }

    bool driveDirect(int16_t leftVelocity, int16_t rightVelocity);

    bool drive(int16_t velocity, int16_t radius);
    bool vacuum(uint8_t pwm) { vacuum_ = pwm; motors(); return true; }
    bool main_brush(uint8_t pwm) { main_brush_ = pwm; motors(); return true; }
    bool side_brush(uint8_t pwm) { side_brush_ = pwm; motors(); return true; }

    private:
      void motors();  // Set speed of main_brush, side_brush and vacuum
      uint8_t main_brush_ = 0;
      uint8_t side_brush_ = 0;
      uint8_t vacuum_ = 0;
};
