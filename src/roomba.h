#pragma once
#include <string>
#include <Arduino.h>
#include <functional>
#include <map>

#include <TinyMqtt.h>
#include "command.h"

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

    uint16_t voltage() const { return voltage_; }
    int16_t current() const { return current_; }
    uint8_t dirt() const { return dirt_; }

    std::string unexpectedChars()
    {
      std::string cpy = unexpected_read_;
      unexpected_read_.clear();
      return cpy;
    }

    using ReadCallback = std::function<void(uint8_t *buf)>;

    bool readBusy() const { return read_left_ ? true : false; }
    void readBytes(int8_t count, ReadCallback callback)
    {
      read_timeout_ = millis() + 200 + (std::abs(count) << 4) * (1000 / 115200);
      read_ptr_ = read_buf_;
      if (count < 0) // fill buffer from end to start
      {
        read_ptr_ = read_ptr_ + std::abs(count) -1;
      }
      read_left_ = count;
      read_cb_ = callback;
      // use can_yield and yield ? to avoid timeout etc ?
    }

    void stopRead()
    {
      read_left_ = 0;
    }

    private:
      struct PeriodicReadBytes
      {
        PeriodicReadBytes(uint16_t& dest, const uint16_t ms, uint8_t packetId)
        : buf(reinterpret_cast<uint8_t*>(&dest)) , len(-2) , ms(ms) , next(millis()+ms) , packetId(packetId) {}

        PeriodicReadBytes(int16_t& dest, const uint16_t ms, uint8_t packetId)
        : buf(reinterpret_cast<uint8_t*>(&dest)) , len(-2) , ms(ms) , next(millis()+ms) , packetId(packetId) {}

        PeriodicReadBytes(uint8_t& dest, const uint16_t ms, uint8_t packetId)
        : buf(reinterpret_cast<uint8_t*>(&dest)) , len(sizeof(dest)) , ms(ms) , next(millis()+ms) , packetId(packetId) {}
 
        uint8_t* buf;     // destination
        int8_t len;       // len of destination, negative for reverse order fill
        uint16_t ms;      // delay between reads in ms
        unsigned long next;
        uint8_t packetId; // Id of packet to read (command 142)
      };

      void motors();         // Set speed of main_brush, side_brush and vacuum

      uint8_t main_brush_ = 0;
      uint8_t side_brush_ = 0;
      uint8_t vacuum_ = 0;
      // uint8_t* read_buf_ = nullptr;
      unsigned long read_timeout_ = 0;    // millis() when stop trying to read
      uint8_t read_buf_[128];             // temporary destination
      uint8_t* read_ptr_;
      int8_t read_left_ = 0;             // bytes left to read, negative=> read in reverse order
      uint16_t read_errors_ = 0;
      ReadCallback read_cb_;

      std::string unexpected_read_;
      uint16_t voltage_;
      int16_t current_;
      uint8_t dirt_;
      uint16_t temp_;

    public:
      uint16_t unexpected_bytes_ = 0;   // number of unexpected reveived bytes.
      std::vector<PeriodicReadBytes> periodics_;
      unsigned long received = 0;
};
