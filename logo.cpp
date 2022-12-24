#include "logo.h"

bool Logo::handle_(std::string &str)
{
	return false;
}

void Logo::help_(const std::string& str, HelpStream& out) const
{
	out << "logo commands:" << endl;
}
