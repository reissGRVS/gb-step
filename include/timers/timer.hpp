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

	U16 cntHData;
	U16 prescaler;
	U16 countUp;
	U16 irqEnable;
	U16 timerStart;

	U16 counter;
	U16 reloadValue;
	U32 prescalerCount;

	const std::array<U16, 4> PRESCALER_SELECTION{ 1, 64, 256, 1024 };

	std::shared_ptr<Memory> memory;
};