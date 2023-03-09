#include "roomba_command.h"
#include "roomba.h"

RoombaCommand::RoombaCommand()
{
	handlers = {
	{ "start",   { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.start(); return true; }}},
	{ "off",     { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.start(); return true; }}},
	{ "passive", { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.passive(); return true; }}},
	{ "safe",    { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.safe(); return true; }}},
	{ "full",    { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.full(); return true; }}},
	{ "reset",   { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.reset(); return true; }}},
	{ "stop",    { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.stop(); return true; }}},
	{ "clean",   { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.clean(); return true; }}},
	{ "spot",    { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.spot(); return true; }}},
	{ "dock",    { "" , [](Roomba& roomba, std::string&, OutputStream&)->bool { roomba.dock(); return true; }}},
	// safe mode
	{ "dd",      { "l r : drive direct" , [](Roomba& roomba, std::string& args, OutputStream& out)->bool
							{
								int16_t l=getInt(args);
								int16_t r=getInt(args);
								roomba.driveDirect(l,r);
								return true;
							}}},
	{ "drive",   { "v rad : drive" , [](Roomba& roomba, std::string& args, OutputStream&)->bool
							{
								int16_t l=getInt(args);
								int16_t r=getInt(args);
								roomba.driveDirect(l,r);
								return true;
							}}},
	{ "brush",   { "pwm : speed of main brush" , [](Roomba& roomba, std::string& args, OutputStream&)->bool { return roomba.main_brush(getInt(args)); }}},
	{ "side",    { "pwm : speed of side brush" , [](Roomba& roomba, std::string& args, OutputStream&)->bool { return roomba.side_brush(getInt(args)); }}},
	{ "vacuum",  { "pwm : speed of vacuum"     , [](Roomba& roomba, std::string& args, OutputStream&)->bool { return roomba.vacuum(getInt(args));}}},
	{ "raw",     { "byte byte ... : send raw bytes" , [](Roomba& roomba, std::string& args, OutputStream& out)->bool
			{
				std::string raw;
				std::string::size_type s=0;
				while(s != args.length())
				{
					s = args.length();
					if (s) raw += static_cast<char>(getInt(args));
				}
				if (args.length())
				{
					out << "  garbage (" << args.c_str() << ')' << endl;
				}
				else
				{
					out << "  sending raw bytes, size=" << String(raw.length()) << endl;
					roomba.raw(raw);
				}
				return true;
			}}},
};
}