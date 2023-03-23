#pragma once

#include <string>

class Utils
{

	public:

		virtual const char* name() const = 0;
		virtual void loop() {};

		static std::string firstWord(const std::string&);
		static std::string getWord(std::string&, char sep=' ');
		static void trim(std::string&);
		static int getInt(std::string&);

};
