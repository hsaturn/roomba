#pragma once

class Flash
{
  public:
    Flash(unsigned long interval);

    void flash();
    void loop();
    void error(unsigned long interval) { error_ = interval; }

  private:
    void led();
    unsigned long interval_;
    unsigned long flash_;
    bool led_ = true;
    unsigned long error_ = 0;

};