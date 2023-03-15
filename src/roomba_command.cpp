#include "roomba_command.h"
#include "roomba.h"

RoombaCommand::RoombaCommand()
{
	handlers = {
	{ "start",   { "" , [](Params& p)->bool { p.roomba.start(); return true; }}},
	{ "off",     { "" , [](Params& p)->bool { p.roomba.start(); return true; }}},
	{ "passive", { "" , [](Params& p)->bool { p.roomba.passive(); return true; }}},
	{ "safe",    { "" , [](Params& p)->bool { p.roomba.safe(); return true; }}},
	{ "full",    { "" , [](Params& p)->bool { p.roomba.full(); return true; }}},
	{ "reset",   { "" , [](Params& p)->bool { p.roomba.reset(); return true; }}},
	{ "stop",    { "" , [](Params& p)->bool { p.roomba.stop(); return true; }}},
	{ "clean",   { "" , [](Params& p)->bool { p.roomba.clean(); return true; }}},
	{ "spot",    { "" , [](Params& p)->bool { p.roomba.spot(); return true; }}},
	{ "dock",    { "" , [](Params& p)->bool { p.roomba.dock(); return true; }}},
	// safe mode
	{ "dd",      { "l r : drive direct" , [](Params& p)->bool
							{
								int16_t l=getInt(p.args);
								int16_t r=getInt(p.args);
								p.roomba.driveDirect(l,r);
								return true;
							}}},
	{ "drive",   { "v rad : drive" , [](Params& p)->bool
							{
								int16_t l=getInt(p.args);
								int16_t r=getInt(p.args);
								p.roomba.driveDirect(l,r);
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
			p.out << "received  : " << p.roomba.received << endl;
			p.out << "unexpected: " << p.roomba.unexpected_bytes_ << endl;
			p.out << "uart tx-rx: " << Serial.tx() << '-' << Serial.rx() << endl;
			p.out << "reading ? : " << (p.roomba.readBusy() ? "yes" : "no") << endl;
			return true;
		}}},
	{ "periodics", { "", [](Params& p)->bool
		{
			p.out << "periodics: now=" << millis() << endl;
			for(const auto& periodic: p.roomba.periodics_)
				p.out << "packet " << periodic.packetId << ", ms=" << periodic.ms << ", next=" << periodic.next << endl;
			return true;
		}
	}},
};
}
