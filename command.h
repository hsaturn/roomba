#pragma once

#include <string>
#include <vector>

#include <ESPTelnet.h>
using HelpStream=ESPTelnet;
#define endl "\r\n"

class Command
{
	public:
		virtual const char* name() const = 0;

		static bool handle(std::string &);
		static void addHandler(Command*);
		static std::string firstWord(const std::string&);
		static std::string getWord(std::string&);
		static int getInt(std::string&);
		static void help(std::string&, HelpStream&);

	protected:
		virtual bool handle_(std::string &) = 0;
		virtual void help_(const std::string&, HelpStream&) const = 0;

	private:
		static std::vector<Command*>	commands;
};
