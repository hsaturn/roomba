#pragma once

#include <string>
#include "command.h"

class Logo : public Command
{
	public:
		const char* name() const override { return "logo"; }
};
