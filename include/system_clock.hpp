
#pragma once

#include "int.hpp"

class SystemClock {
public:
	void Tick(U32 ticks) { total += ticks; }

	U32 SinceLastCheck()
	{
		auto since = total - lastCheck;
		lastCheck = total;
		return since;
	}

private:
	U32 total = 0;
	U32 lastCheck = 0;
};