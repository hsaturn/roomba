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
		struct Params
		{
			Params(Roomba& r, std::string& a, OutputStream& o) : roomba(r), args(a), out(o) {};

			Roomba& roomba;
			std::string& args;
			OutputStream& out;
		};

		virtual const char* name() const = 0;
		virtual void loop() {};
		static void loops();

		static bool handle(Params& params);
		static void addHandler(Command*);
		static std::string firstWord(const std::string&);
		static std::string getWord(std::string&);
		static void trim(std::string&);
		static int getInt(std::string&);
		static void help(Params&);

		using HandlerFunc = bool(Params&);
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
