#pragma once

#include "module.h"

struct Every
{
  std::string cmd;
  uint32_t ms;
  uint32_t next;
  bool active=true;

  void dump(OutputStream& out) const
  {
    out << (active ? "enabled " : "disabled ");

    auto mill=millis();
    out << ms << "ms [" << cmd.c_str() << "] next in ";
    if (mill > next)
      out << "now";
    else
      out << next-mill << "ms";
  }
};


class Nanos : public Module
{
  public:
    Nanos();
    const char* name() const override { return "nanos"; }

    void loop() override;
    std::string getEvery();

    void addModule(Module *);
    void loops();

    bool execute(Params& params);
    void help(Params& params);

  private:
    void def(Params&);
    void undef(Params&);
    char recurse = 0;
    bool every_on_ = true;

    void every(Params&);

    std::vector<Every> every_;

  private:
    std::vector<Module*> modules;
};

extern Nanos* nanos;