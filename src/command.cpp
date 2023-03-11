#include "command.h"

std::vector<Command*> Command::commands;

bool Command::handle(Params& p)
{
	while(p.args.length())
	{
		std::string module="";
		std::string cmd = getWord(p.args);
		std::string::size_type dot = cmd.find('.');
		if (dot != std::string::npos)
		{
			module = cmd.substr(0, dot);
			cmd.erase(0, dot+1);
		}
		if (cmd == "help")
		{
			help(p);
			continue;
		}
		else if (cmd == "modules")
		{
			p.out << "Installed modules:";
			for (auto& command: commands)
				p.out << ' ' << command->name();
			p.out << '.' << endl;
			continue;
		}

		bool exec = false;
		for(auto& command: commands)
		{
			// p.out << "exec(" << cmd.c_str() << ") args=(" << p.args.c_str() << ')' <<endl;
			if (module.length() and module != command->name()) continue;

			auto it=command->handlers.find(cmd.c_str());
			if (it == command->handlers.end()) continue;
			exec = true;
			it->second.handler(p);
			break;
		}
		if (not exec)
			return false;
	}
	return true;
}

void Command::addHandler(Command* command)
{
	commands.push_back(command);
}

void Command::help(Params& p)
{
	p.out << endl;
	p.out << "modules       : list of installed modules" << endl;
	p.out << "help [module] : this help" << endl;
	p.out << "  params: l/r/v=-500 .. 500mm/s  rad=-2000 2000mm" << endl;
	p.out << endl;
	auto cmd = getWord(p.args);
	for(auto& command: commands)
	{
		if (command->handlers.size() and (cmd.length()==0 or cmd==command->name()))
		{
			p.out << command->name() << " commands:" << endl;
			for(auto& it: command->handlers)
			{
				p.out << "  " << it.first.c_str() << ' ' << it.second.args.c_str() << endl;
			}
			p.out << endl;
		}
	}
}

void Command::loops()
{
	for(auto& command: commands)
	{
		command->loop();
	}	
}

std::string Command::firstWord(const std::string& str)
{
	auto spc = str.find(' ');
	if (spc == std::string::npos)
		spc = str.length();
	return str.substr(0, spc);
}

std::string Command::getWord(std::string& str)
{
	std::string word;
	auto spc = str.find(' ');
	if (spc == std::string::npos)
	{
		word = str;
		str = "";
	}
	else
	{
		word = str.substr(0, spc);
		str = str.substr(spc+1);
	}
	return word;
}

void Command::trim(std::string& str)
{
	while(str[0]==' ') str.erase(0,1);
}

int Command::getInt(std::string& str)
{
	int value = 0;
	int base = 10;
	bool minus = str[0]=='-';
	bool valid = false;
	if (str.length()>2 and str[0]=='0' and str[1]=='x')
	{
		base = 16;
		str.erase(0,2);
	}
	std::string::size_type pos = minus ? 1 : 0;
	while(pos < str.size() and
	 ((str[pos]>='0' and str[pos]<='9') or (base==16 and str[pos]>='A' and str[pos]<='F')))
	{
		valid = true;
		if (str[pos]<='9')
			value = value*base + str[pos]-'0';
		else
			value = value*base + str[pos]-'A' + 10;

		pos++;
	}
	if (not valid) return 0;
	str.erase(0,pos);
	trim(str);
	return minus ? -value : value;
}