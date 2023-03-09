#pragma once

#include <string>
#include "command.h"

class Nanos : public Command
{
	public:
		Nanos();
		const char* name() const override { return "nanos"; }
};
