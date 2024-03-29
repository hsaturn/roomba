#include "lidar.h"
#include <TinyMqtt.h>

Lidar::Lidar(OutputStream& out, MqttClient* clt)
  : mqtt(clt)
{
 handlers = {
    { "mm", { "", [this](Params& p)->bool
      {
        p.out << "Lidar: ";
        if (last_update)
        {
          p.out<< mm() << "mm " << (millis()-last_update) << "ms ago";
        }
        else
          p.out << "error !";
        p.out << endl;
        return true;
      }}},
    { "stop", { "", [this](Params& p)->bool { stop(); return true; }}},
    { "init", { "", [this](Params& p)->bool
      {
        init();
        p.out << "init " << (last_update ? "ok" : "failed") << endl;
        return true;
      }
    }},
  };
}

void Lidar::stop()
{
  sensor.stopContinuous();
  Wire.endTransmission();
}

void Lidar::init()
{
  Wire.endTransmission();
  delay(100);
  Wire.begin();
  Wire.setClock(400000);
  delay(100);
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    last_update = 0;
    return;
  }
  last_update = millis();

  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(500);
  sensor.startContinuous(50);
 }

void Lidar::loop()
{
  if (last_update == 0) return;
  if (sensor.dataReady())
  {
    sensor.read();
    if (sensor.ranging_data.range_status == 0)
    last_update = millis();
    distance = sensor.ranging_data.range_mm;
    if (mqtt)
    {
      static Topic lidar("lidar");
      mqtt->publish(lidar, String(distance));
    }
  } 
}