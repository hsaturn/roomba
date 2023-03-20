#include "roomba_command.h"
#include "roomba.h"
#include "uart.h"

RoombaCommand::RoombaCommand()
{
	handlers = {
	{ "start",   { "" , [](Params& p)->bool { p.roomba.start(); return true; }}},
	{ "off",     { "" , [](Params& p)->bool { p.roomba.start(); return true; }}},
	{ "passive", { "" , [](Params& p)->bool { p.roomba.passive(); return true; }}},
	{ "safe",    { "" , [](Params& p)->bool { p.roomba.safe();  return true; }}},
	{ "full",    { "" , [](Params& p)->bool { p.roomba.full();  return true; }}},
	{ "reset",   { "" , [](Params& p)->bool { p.roomba.reset(); return true; }}},
	{ "stop",    { "" , [](Params& p)->bool { p.roomba.stop();  return true; }}},
	{ "clean",   { "" , [](Params& p)->bool { p.roomba.clean(); return true; }}},
	{ "spot",    { "" , [](Params& p)->bool { p.roomba.spot();  return true; }}},
	{ "dock",    { "" , [](Params& p)->bool { p.roomba.dock();  return true; }}},
	{ "batt",    { "" , [](Params& p)->bool
		{
			p.out << "voltage: " << p.roomba.voltage() << "mv" << endl;
			p.out << "current: " << p.roomba.current() << "mA" << endl;
			return true;
		}}},
	{ "dirt",    { "" , [this](Params& p)->bool { p.out << "dirt: " << (int)p.roomba.dirt() << endl; return true; }}},
	{ "dd",      { "l r : drive direct" , [this](Params& p)->bool
							{
								int16_t l=getInt(p.args);
								int16_t r=getInt(p.args);
								p.roomba.driveDirect(l,r);
								return true;
							}}},
	{ "dd+",      { "l r : NUI" , [this](Params& p)->bool
							{
								p.roomba.lvelocity_ += getInt(p.args);
								p.roomba.rvelocity_ += getInt(p.args);
								p.roomba.driveDirect(p.roomba.lvelocity_, p.roomba.rvelocity_);
								return true;
							}}},
	{ "drive",   { "v rad : drive" , [](Params& p)->bool
							{
								int16_t v=getInt(p.args);
								int16_t a=getInt(p.args);
								p.roomba.drive(v,a);
								return true;
							}}},
	{ "brush",   { "pwm : speed of main brush" , [](Params& p)->bool { return p.roomba.main_brush(getInt(p.args)); }}},
	{ "side",    { "pwm : speed of side brush" , [](Params& p)->bool { return p.roomba.side_brush(getInt(p.args)); }}},
	{ "vacuum",  { "pwm : speed of vacuum"     , [](Params& p)->bool { return p.roomba.vacuum(getInt(p.args));}}},
	{ "raw",     { "byte byte ... : send raw bytes" , [](Params& p)->bool
			{
				std::string raw;
				std::string::size_type s=0;
				while(s != p.args.length())
				{
					s = p.args.length();
					if (s) raw += static_cast<char>(getInt(p.args));
				}
				if (p.args.length())
				{
					p.out << "  garbage (" << p.args.c_str() << ')' << endl;
				}
				else
				{
					p.out << "  sending raw bytes, size=" << String(raw.length()) << endl;
					p.roomba.raw(raw);
				}
				return true;
			}}},
	{ "status",  { "", [](Params& p)
		{
			p.out << "mqtt sent  : " << p.roomba.sent_ << endl;
			p.out << "timeout rcv: " << p.roomba.timeouts_ << endl;
			p.out << "received   : " << p.roomba.received << endl;
			p.out << "unexpected : " << p.roomba.unexpected_bytes_ << endl;
			p.out << "reading ?  : " << (p.roomba.readBusy() ? "yes" : "no") << endl;
			return true;
		}}},
	{ "periodics", { "[topic ms]", [](Params& p)->bool
		{
			p.out << "periodics: now=" << millis() << ", args=(" << p.args << ')' << endl;
			Topic topic=getWord(p.args);
			int ms = getInt(p.args);
			for(auto& periodic: p.roomba.periodics_)
			{
				if (topic.matches(periodic.topic))
				{
					periodic.ms = ms;
					periodic.next = 0;
					p.out << "Changed to " << ms << endl;
				}
				p.out << "packet " << periodic.packetId << ", ms=" << periodic.ms
						  << ", next=" << periodic.next << ", topic=" << periodic.topic << endl;
			}
			return true;
		}
	}},
};
}
