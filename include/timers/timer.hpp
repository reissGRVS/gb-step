#pragma once

#include <array>

#include "int.hpp"

struct Timer {

	Timer(U8 ID, U32 CNT_L);

	void UpdateCntH(U16 value);
	U32 CounterUpdate(U32 ticks);
	U32 PrescalerTimingUpdate(const U32& ticks);
	U32 Update(const U32& ticks, const U32& prevOverflow);

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
	U32 ticksLeft = 0x1000;

	const std::array<U16, 4> PRESCALER_SELECTION { 1, 64, 256, 1024 };
};