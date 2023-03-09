#include "flash.h"
#include <Arduino.h>

Flash::Flash(unsigned long interval)
  : interval_(interval),
    flash_(millis())
{
  pinMode(LED_BUILTIN, OUTPUT);
  led();
}

void Flash::loop()
{
  if (millis() > flash_)
  {
    led_ = not led_;
    led();
    if (error_)
      flash_ += error_;
    else
      flash_ += led_ ? 50 : interval_ - 100;
  }
}

void Flash::flash()
{
  led_ = true;
  led();
  flash_ = millis() + 100;
}

void Flash::led()
{
    digitalWrite(LED_BUILTIN, led_ ? LOW : HIGH);
}