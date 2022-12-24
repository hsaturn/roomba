#pragma once

#include <string>
#include "command.h"

class Logo : public Command
{
	public:
		const char* name() const override { return "logo"; }
	protected:
		bool handle_(std::string &) override;
		void help_(const std::string&, HelpStream&) const override;
};
