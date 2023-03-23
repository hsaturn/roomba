#pragma once
#include <string>
#include <Arduino.h>
#include <functional>
#include <map>

#include <TinyMqtt.h>
#include "module.h"

#define BRC_PIN 14

class Roomba
{
  public:
    Roomba();

    void loop(MqttClient*, OutputStream*);

    void start();
    void off()      { Serial.write(133); }
    void clean()    { Serial.write(135); }
    void max()      { Serial.write(136); }
    void spot()     { Serial.write(134); }
    void seekDock() { Serial.write(143); }
    void reset()    { Serial.write(7);   }
    void stop()     { Serial.write(173); }
    void dock()     { Serial.write(143); }
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
      read_timeout_ = millis() + 100;  // 50ms to receive message max
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
      ++timeouts_;
      read_left_ = 0;
    }

    private:
      struct PeriodicReadBytes
      {
        PeriodicReadBytes(uint16_t& dest, const uint16_t ms, uint8_t packetId, const char* topic = nullptr)
        : PeriodicReadBytes(reinterpret_cast<uint8_t*>(&dest), -2, ms, packetId, topic) {}

        PeriodicReadBytes(int16_t& dest, const uint16_t ms, uint8_t packetId, const char* topic = nullptr)
        : PeriodicReadBytes(reinterpret_cast<uint8_t*>(&dest), -2, ms, packetId, topic) {}

        PeriodicReadBytes(uint8_t& dest, const uint16_t ms, uint8_t packetId, const char* topic = nullptr)
        : PeriodicReadBytes(reinterpret_cast<uint8_t*>(&dest), sizeof(dest), ms, packetId, topic) {}

        uint8_t* buf;     // destination
        int8_t len;       // len of destination, negative for reverse order fill
        uint16_t ms;      // delay between reads in ms
        unsigned long next;
        uint8_t packetId; // Id of packet to read (module 142)
        const char* topic;

      private:
        PeriodicReadBytes(uint8_t* buf, int8_t len, uint16_t ms, uint8_t packetId, const char* topic)
        : buf(buf) , len(len) , ms(ms) , next(millis()+ms) , packetId(packetId), topic(topic) {}

      };

      void motors();         // Set speed of main_brush, side_brush and vacuum

    public:
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

      // battery
      std::string unexpected_read_;
      uint16_t voltage_;
      uint16_t capacity_;
      int16_t current_;

      // sensors
      uint8_t dirt_;
      uint8_t buttons_;
      uint8_t charge_;
      uint8_t temp_;
      uint16_t wall_;
      uint16_t lclift_;
      uint16_t flclift_;
      uint16_t frclift_;
      uint16_t rclift_;
      uint8_t bumpers_;   // b7-b0 = res - res - right - fr right | center right - center left - front left - left
      uint16_t bump_l_;
      uint16_t bump_fl_;
      uint16_t bump_cl_;
      uint16_t bump_cr_;
      uint16_t bump_fr_;
      uint16_t bump_r_;

      uint8_t stasis_;


      // velocity
      uint16_t velocity_;
      uint16_t lvelocity_;
      uint16_t rvelocity_;
      uint16_t radius_;

      // steps
      uint16_t lsteps_;
      uint16_t rsteps_;

      uint16_t unexpected_bytes_ = 0;   // number of unexpected reveived bytes.
      using Periodics = std::vector<PeriodicReadBytes>;
      Periodics periodics_;
      Periodics::iterator looper_;
      unsigned long ms_begin_ = 0;  // us when looper_ == begin
      unsigned long last_ploop_ = 0;  // last periodic loop

      unsigned long received = 0;
      unsigned int sent_ = 0;
      unsigned long timeouts_ = 0; // receive timeouts
};
