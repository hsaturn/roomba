#include "command.h"

std::vector<Command*> Command::commands;

bool Command::handle(std::string &str)
{
	for(auto& command: commands)
		if (command->handle_(str))
			return true;
	return false;
}

void Command::addHandler(Command* command)
{
	commands.push_back(command);
}

void Command::help(std::string& topic, HelpStream& out)
{
	for(auto& command: commands)
		command->help_(topic, out);
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
