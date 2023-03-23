#include "module.h"

std::string Utils::firstWord(const std::string& str)
{
	auto spc = str.find(' ');
	if (spc == std::string::npos)
		spc = str.length();
	return str.substr(0, spc);
}

std::string Utils::getWord(std::string& str, char sep)
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

void Utils::trim(std::string& str)
{
	while(str[0]==' ') str.erase(0,1);
}

int Utils::getInt(std::string& str)
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