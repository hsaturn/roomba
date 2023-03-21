#include "command.h"

std::vector<Command*> Command::commands;

bool Command::handle(Params& p)
{
	while(p.args.length())
	{
		std::string module="";
		std::string cmd = getWord(p.args);
		std::string::size_type slash = cmd.find('/');
		if (slash != std::string::npos)
		{
			module = cmd.substr(0, slash);
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
			for (auto& command: commands)
				p.out << ' ' << command->name();
			p.out << '.' << endl;
			continue;
		}

		std::vector<std::string> candidates;
		Handler* found = nullptr;
		for(auto& command: commands)
		{
			if (module.length() and module != command->name()) continue;

			if (module.length())
			{
				// prefix with module name makes the cmd unambiguous
				auto it=command->handlers.find(cmd.c_str());
				if (it != command->handlers.end())
				{
					found = &(it->second);
					break;
				}
			}

			for(auto& it: command->handlers)
			{
				if (it.first.substr(0, cmd.length()) == cmd)
				{
					candidates.push_back(std::string(command->name())+'/'+it.first);
					found = &it.second;
				}
			}
		}

		if (candidates.size() > 1)
		{
			p.out << endl << "Ambiguous command '" << cmd.c_str() << "', candidates are : " << endl;
			for (const auto &candidate : candidates)
			{
				p.out << "  " << candidate << endl;
			}
			p.out << endl;
			return false;
		}
		else if (not found)
			return false;
		
		found->handler(p);
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
	if (p.args.length() == 0)
	{
		p.out << "modules          : list of installed modules" << endl;
		p.out << "help or ?        : this help" << endl;
		p.out << "help or ? module : help on module" << endl;
		p.out << "help command     : search command (* for all)" << endl;
		p.out << endl;
		p.out << "command can be prefix with module name, ex: roomba/clean" << endl;
		p.out << endl;
	}

	auto cmd = getWord(p.args);
	if (cmd.length()==0) return;

	uint8_t found = 0;
	for(auto& command: commands)
	{
		if (command->handlers.size() and cmd==command->name())
		{
			p.out << command->name() << " commands:" << endl;
			for(auto& it: command->handlers)
			{
				found++;
				p.out << "  " << it.first.c_str() << ' ' << it.second.args.c_str() << endl;
			}
			p.out << endl;
		}
		else
		{
			auto it = command->handlers.find(cmd);
			if (it != command->handlers.end())
			{
				p.out << command->name() << '/' << it->first.c_str() << ' ' << it->second.args.c_str() << endl;
				found++;
				break;
			}

			// Search approximative commands
			for(auto it: command->handlers)
			{
				if (cmd=="*" or cmd==it.first.substr(0, cmd.length()))
				{
					p.out << command->name() << '/' << it.first.c_str() << ' ' << it.second.args.c_str() << endl;
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

std::string Command::getWord(std::string& str, char sep)
{
	std::string word;
	auto spc = str.find(sep);
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