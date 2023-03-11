#pragma once

#include "command.h"

class Nanos : public Command
{
	public:
		Nanos();
		const char* name() const override { return "nanos"; }

	private:
		void def(Params&);
		void undef(Params&);
		char recurse = 0;
};
