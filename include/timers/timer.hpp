#pragma once

#include "int.hpp"
#include "memory/memory.hpp"
#include "utils.hpp"

struct Timer {

	Timer(U8 ID, U32 CNT_L, std::shared_ptr<Memory> memory);

	void UpdateCntH(U16 value);
	U32 CounterUpdate(U32 ticks);
	U32 PrescalerTimingUpdate(U32 ticks);
	U32 Update(U32 ticks, U32 prevOverflow);

	const U8 ID;
	const U32 CNT_L;
	const U32 CNT_H;

	U16 cntHData = 0;
	U16 prescaler = 0;
	U16 countUp = 0;
	U16 irqEnable = 0;
	U16 timerStart = 0;

	U16 counter = 0;
	U16 reloadValue = 0;
	U32 prescalerCount = 0;

	const std::array<U16, 4> PRESCALER_SELECTION{ 1, 64, 256, 1024 };

	std::shared_ptr<Memory> memory;
};