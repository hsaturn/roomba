#include "nanos.h"
#include "flash.h"
#include <ESP8266WiFi.h>
#include <TinyStreaming.h>

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
        { "echo",   { "" ,                  [](Params& p)->bool { p.out << p.args.c_str() << endl; p.args.clear(); return true; }}},
        { "ip",     { "" ,                  [](Params& p)->bool { p.out << "ip: " << WiFi.localIP().toString() << endl; return true; }}},
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
        { "pwr" ,  { "save full",    [this](Params& p)->bool { pwr_policy(p); return true; }}},
        { "loops", { " : avg loop time",[this](Params& p)->bool {
            p.out << "Avg loop time: " << String(loops_avg_) << "us, min/max=" << min_loop_ << '/' << max_loop_ << "us." << endl;
            min_loop_ = 99999999;
            max_loop_ = 0;
            return true;
        }}},
        { "every", { "ms command | on | off | list | remove #", [this](Params& p)->bool { this->every(p); return true; }}},
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
        { "toggle", { "" , [](Params& p)->bool
            {
                uint8_t pin = getInt(p.args);
                digitalWrite(pin, digitalRead(pin) == LOW ? HIGH : LOW);
                return true;
            }}},
        { "rev", { "" , [](Params&p) -> bool { p.out << "Esp firmware: " << AUTO_VERSION << endl; }}},
    };
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

void Nanos::loop()
{
    auto ms = millis();
    if (last_loop_)
    {
        auto delta_ms = ms-last_loop_;
        loops_avg_ = 0.99*loops_avg_ + 0.01*delta_ms;
        max_loop_ = std::max(delta_ms, max_loop_);
        min_loop_ = std::min(delta_ms, min_loop_);
    }
    last_loop_ = ms;
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