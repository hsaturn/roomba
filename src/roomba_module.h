#pragma once

#include <map>
#include <string>

#include "Command.h"

class RoombaCommand : public Command
{
	public:
		RoombaCommand();
		const char *name() const override { return "roomba"; }

};
