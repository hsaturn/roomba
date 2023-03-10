#include "nanos.h"
#include "flash.h"
#include <ESP8266WiFi.h>

extern Flash flash;

void pwr_policy(Command::Params& p)
{
    Command::getWord(p.args);
    p.out << "NYI" << endl;
}

Nanos::Nanos()
{
    handlers = {
        { "reboot", { "" ,                  [](Params& p)->bool { ESP.reset(); return true; }}},
        { "free", { "" ,                    [](Params& p)->bool { p.out << "free heap: " << String(ESP.getFreeHeap()) << endl; return true; }}},
        { "error", { "n : set flash mode" , [](Params& p)->bool { flash.error(getInt(p.args)); return true; }}},
        { "int", { "" ,                     [](Params& p)->bool { 
            std::string::size_type s=0;
            while(s != p.args.length())
            {
                s = p.args.length();
                p.out << "int(" << p.args.c_str() << ")=" << String(getInt(p.args));
                p.out << ", p.args=(" << p.args.c_str() << ")" << endl;
            } return true;
        }}},
        { "def",   { "name command", [this](Params& p)->bool { this->def(p); return true; } }},
        { "undef", { "name" ,        [this](Params& p)->bool { this->undef(p); return true; } }},
        { "wait",  { "ms",           [this](Params& p)->bool { delay(getInt(p.args)); return true; }}},
        { "pwr" ,  { "low high",     [this](Params& p)->bool { pwr_policy(p); return true; }}},
    };
}

void Nanos::def(Params& p)
{
    std::string name = getWord(p.args);
    std::string cmd = p.args;
    handlers[name] = { "", [this, cmd](Params& q) -> bool
        {
            if (++recurse < 20)
            {
                q.args = cmd;
                std::string tmp(q.args);
                q.out << "exec " << tmp.c_str() << endl;
                Command::handle(q);
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
        p.out << "Unknown command: '" << cmd.c_str() << '\'' << endl;
    else
        handlers.erase(cmd);
}