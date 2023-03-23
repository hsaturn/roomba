#include "nanos.h"
#include "flash.h"
#include <ESP8266WiFi.h>
#include <TinyStreaming.h>

extern Flash flash;

void pwr_policy(Module::Params& p)
{
    Utils::getWord(p.args);
    p.out << "NYI" << endl;
}

void Nanos::addModule(Module* module)
{
    modules.push_back(module);
}

void Nanos::loops()
{
  for(auto& module: modules)
  {
    module->loop();
  }  
}

Nanos::Nanos()
{
  addModule(this);
  handlers = {
      { "reboot", { "" ,                  [](Params& p)->bool { ESP.reset(); return true; }}},
      { "echo",   { "" ,                  [](Params& p)->bool { p.out << p.args.c_str() << endl; p.args.clear(); return true; }}},
      { "ip",     { "" ,                  [](Params& p)->bool { p.out << "ip: " << WiFi.localIP().toString() << endl; return true; }}},
      { "free",   { "" ,                  [](Params& p)->bool { p.out << "free heap: " << String(ESP.getFreeHeap()) << endl; return true; }}},
      // FIXME flash could be a module (led ?)
      { "flash",  { "ms : set flash led", [](Params& p)->bool { flash.error(getInt(p.args)); return true; }}},
      { "int", { "" ,                     [](Params& p)->bool { 
          std::string::size_type s=0;
          while(s != p.args.length())
          {
              s = p.args.length();
              p.out << "int(" << p.args.c_str() << ")=" << String(getInt(p.args));
              p.out << ", p.args=(" << p.args.c_str() << ")" << endl;
          } return true;
      }}},
      { "def",   { "name module", [this](Params& p)->bool { this->def(p); return true; } }},
      { "undef", { "name" ,        [this](Params& p)->bool { this->undef(p); return true; } }},
      { "wait",  { "ms",           [this](Params& p)->bool { delay(getInt(p.args)); return true; }}},
      { "pwr" ,  { "save full",    [this](Params& p)->bool { pwr_policy(p); return true; }}},
      { "every", { "ms module | on | off | list | remove #", [this](Params& p)->bool { this->every(p); return true; }}},
      { "pin", { "n , read | in | ind | fun | out" , [](Params& p)->bool
          {
              uint8_t pin = getInt(p.args);
              std::string mode = getWord(p.args);
              p.out << "pin " << pin << ' ';
              if (mode == "read") { p.out << (digitalRead(pin)==LOW ? "low" : "high"); }
              else if (mode == "fun") { pinMode(pin, FUNCTION_3); p.out  << ": function_3"; }
              else if (mode == "in") { pinMode(pin, INPUT); p.out << ": input"; }
              else if (mode =="ind") { pinMode(pin, INPUT_PULLDOWN_16); p.out << ": input pulldown"; }
              else if (mode == "out") { pinMode(pin, OUTPUT); p.out << ": output"; }
              else { p.out << '?'; }
              p.out << endl;
              return true;
          }}},
      { "toggle", { "pin" , [](Params& p)->bool
          {
              uint8_t pin = getInt(p.args);
              digitalWrite(pin, digitalRead(pin) == LOW ? HIGH : LOW);
              return true;
          }}},
      { "rev", { "" , [](Params&p) -> bool { p.out << "Esp firmware: " << AUTO_VERSION << endl; return true; }}},
  };
}

bool Nanos::execute(Params& p)
{
  while(p.args.length())
  {
    std::string module_name="";
    std::string cmd = getWord(p.args);
    std::string::size_type slash = cmd.find('/');
    if (slash != std::string::npos)
    {
      module_name = cmd.substr(0, slash);
      cmd.erase(0, slash+1);
    }
    if (cmd == "help" or cmd=="?")
    {
      help(p);
      continue;
    }
    else if (cmd == "modules")
    {
      p.out << "Installed modules:";
      for (auto& module: modules)
        p.out << ' ' << module->name();
      p.out << '.' << endl;
      continue;
    }

    std::vector<std::string> candidates;
    Handler* found = nullptr;
    for(auto& module: modules)
    {
      if (module_name.length() and module_name != module->name()) continue;

      if (module_name.length())
      {
        // prefix with module name makes the cmd unambiguous
        auto it=module->handlers.find(cmd.c_str());
        if (it != module->handlers.end())
        {
          found = &(it->second);
          break;
        }
      }

      for(auto& it: module->handlers)
      {
        if (it.first.substr(0, cmd.length()) == cmd)
        {
          candidates.push_back(std::string(module->name())+'/'+it.first);
          found = &it.second;
        }
      }
    }

    if (candidates.size() > 1)
    {
      if (cmd.length())
        p.out << endl << "Ambiguous module '" << cmd.c_str() << "', candidates are : " << endl;
      for (const auto &candidate : candidates)
      {
        p.out << "  " << candidate << endl;
      }
      p.out << endl;
      if (cmd.length()) return false;
    }
    else if (not found)
      return false;
    
    found->handler(p);
  }
  return true;
}

void Nanos::help(Params& p)
{
  p.out << endl;
  if (p.args.length() == 0)
  {
    p.out << "modules          : list of installed modules" << endl;
    p.out << "help or ?        : this help" << endl;
    p.out << "help or ? module : help on module" << endl;
    p.out << "help module     : search module (* for all)" << endl;
    p.out << endl;
    p.out << "module can be prefix with module name, ex: roomba/clean" << endl;
    p.out << endl;
  }

  auto cmd = getWord(p.args);
  if (cmd.length()==0) return;

  uint8_t found = 0;
  for(auto& module: modules)
  {
    if (module->handlers.size() and cmd==module->name())
    {
      p.out << module->name() << " modules:" << endl;
      for(auto& it: module->handlers)
      {
        found++;
        p.out << "  " << it.first.c_str() << ' ' << it.second.args.c_str() << endl;
      }
      p.out << endl;
    }
    else
    {
      auto it = module->handlers.find(cmd);
      if (it != module->handlers.end())
      {
        p.out << module->name() << '/' << it->first.c_str() << ' ' << it->second.args.c_str() << endl;
        found++;
        break;
      }

      // Search approximative modules
      for(auto it: module->handlers)
      {
        if (cmd=="*" or cmd==it.first.substr(0, cmd.length()))
        {
          p.out << module->name() << '/' << it.first.c_str() << ' ' << it.second.args.c_str() << endl;
          found++;
        }
      }
    }
  }
  if (found)
  {
    p.out << endl << "params: l/r/v=-500 .. 500mm/s  rad=-2000 2000mm" << endl;
  }
}

void Nanos::every(Params& p)
{
    p.out << "every: " << p.args.c_str() << endl;
    std::string cmd = getWord(p.args);
    if (cmd == "on")
    {
        every_on_ = true;
    }
    else if (cmd == "off")
    {
        every_on_ = false;
    }
    else if (cmd == "remove")
    {
        uint32_t index = getInt(p.args);
        if (index < every_.size())
            every_.erase(every_.begin()+index);
    }
    else if (cmd == "list")
    {
        int index=0;
        p.out << "Every list, size=" << every_.size() << endl;
        p.out << "------------" << endl;
        for(const auto& e: every_)
        {
            p.out << '#' << index++ << ' ';
            e.dump(p.out);
            p.out << endl;
        }
        p.out << "------------" << endl;
    }
    else
    {
        int delay = getInt(cmd);
        if (delay == 0)
        {
            p.out << "ERROR : expecting number" << endl;
            return;
        }
        Every e;
        e.active = true;
        e.cmd = p.args;
        e.ms = delay;
        e.next = millis() + delay;
        every_.push_back(e);
    }
}

void Nanos::def(Params& p)
{
    std::string name = getWord(p.args);
    std::string cmd = p.args;
    p.out << "cmd=(" << cmd.c_str() << ")" << endl;
    handlers[name] = { "", [this, cmd](Params& q) -> bool
        {
            if (++recurse < 20)
            {
                q.args = cmd+' '+q.args;
                execute(q);
            }
            else
                q.out << "error: too many recursive calls" << endl;
            recurse--;
            return true;
        }};
    p.args = "";
}

void Nanos::undef(Params& p)
{
    std::string cmd = getWord(p.args);
    if (handlers.find(cmd) == handlers.end())
        p.out << "Unknown module: '" << cmd.c_str() << '\'' << endl;
    else
        handlers.erase(cmd);
}

void Nanos::loop()
{
}

std::string Nanos::getEvery()
{
    auto ms = millis();
    for(auto& e: every_)
    {
        if (ms >= e.next)
        {
            e.next += e.ms;
            return e.cmd;
        }
    }
    return "";
}
