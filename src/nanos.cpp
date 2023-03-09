#include "nanos.h"
#include "flash.h"

extern Flash flash;

Nanos::Nanos()
{
    handlers = {
        { "reboot", { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { ESP.reset(); return true; }}},
        { "free", { "" , [](Roomba& roomba, std::string&, OutputStream& out)->bool { out << "free heap: " << String(ESP.getFreeHeap()) << endl; return true; }}},
        { "error", { "n : set flash mode" , [](Roomba& roomba, std::string& args, OutputStream&)->bool { flash.error(getInt(args)); return true; }}},
        { "int", { "" , [](Roomba& roomba, std::string& args, OutputStream& out)->bool { 
            std::string::size_type s=0;
            while(s != args.length())
            {
                s = args.length();
                out << "int(" << args.c_str() << ")=" << String(getInt(args));
                out << ", args=(" << args.c_str() << ")" << endl;
            } return true;
        }}},
    };
}