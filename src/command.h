#pragma once

#include <string>
#include <vector>
#include <map>

#include <ESPTelnet.h>
using HelpStream=ESPTelnet;
using OutputStream=ESPTelnet;
#define endl "\r\n"
class Roomba;

class Command
{

	public:
		virtual const char* name() const = 0;

		static bool handle(std::string &, Roomba&, OutputStream& out);
		static void addHandler(Command*);
		static std::string firstWord(const std::string&);
		static std::string getWord(std::string&);
		static void trim(std::string&);
		static int getInt(std::string&);
		static void help(std::string&, HelpStream&);

		using HandlerFunc = bool(Roomba &, std::string &str, OutputStream &);
		struct Handler
		{
			std::string args;
			std::function<HandlerFunc> handler;
		};

		using command = std::string;
		std::map<command, Handler> handlers;

	private:
		static std::vector<Command*>	commands;
};
