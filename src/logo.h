#pragma once

#include "module.h"

class Logo : public Module
{
	public:
		const char* name() const override { return "logo"; }
};
