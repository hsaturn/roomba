#pragma once

#include <string>
#include <vector>
#include <map>
#include "utils.h"

#include <ESPTelnet.h>
using OutputStream=ESPTelnet;
using HelpStream=ESPTelnet;
#include <TinyStreaming.h>

inline OutputStream& operator <<(OutputStream& obj, _EndLineCode) { obj << "\r\n"; return obj; }

inline ESPTelnet &operator <<(ESPTelnet &obj, std::string arg) { obj.print(arg.c_str()); return obj; } 

class Roomba;

class Module
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
		virtual void loop() {}
		static void loops();

		static inline std::string firstWord(const std::string& s) { return Utils::firstWord(s); }
		static inline std::string getWord(std::string& s, char sep=' ') { return Utils::getWord(s, sep); }
		static inline void trim(std::string& s) { Utils::trim(s); }
		static inline int getInt(std::string& s) { return Utils::getInt(s); }

		using HandlerFunc = bool(Params&);
		struct Handler
		{
			std::string args;
			std::function<HandlerFunc> handler;
		};

		using command = std::string;
		using Handlers = std::map<command, Handler>;
		Handlers handlers;
};
