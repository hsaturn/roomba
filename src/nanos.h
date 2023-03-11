#pragma once

#include "command.h"

class Nanos : public Command
{
	public:
		Nanos();
		const char* name() const override { return "nanos"; }

		void loop() override;

	private:
		void def(Params&);
		void undef(Params&);
		char recurse = 0;
		float loops_avg_ = 0;	// avg delay between loops in us
		unsigned long last_loop_ = 0;
		unsigned long min_loop_ = 999999;
		unsigned long max_loop_ = 0;
};
