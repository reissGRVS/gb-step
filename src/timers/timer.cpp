#include "timers/timer.hpp"

Timer::Timer(U8 ID, U32 CNT_L, std::shared_ptr<Memory> memory)
	: ID(ID)
	, CNT_L(CNT_L)
	, CNT_H(CNT_L + 2)
	, memory(memory)
{
}

void Timer::UpdateCntH(U16 value)
{
	cntHData = value;
	prescaler = BIT_RANGE(cntHData, 0, 1);
	countUp = BIT_RANGE(cntHData, 2, 2);
	irqEnable = BIT_RANGE(cntHData, 6, 6);
	timerStart = BIT_RANGE(cntHData, 7, 7);
}

U32 Timer::CounterUpdate(U32 ticks)
{
	U32 overflow = 0;
	if (ticksLeft <= ticks) {
		ticks -= ticksLeft;
		auto tickRange = 0x10000 - reloadValue;
		overflow += 1 + (ticks / tickRange);
		counter = reloadValue + (ticks % tickRange);
	} else {
		counter += ticks;
	}

	ticksLeft = 0x10000u - counter;

	return overflow;
}

U32 Timer::PrescalerTimingUpdate(const U32& ticks)
{
	prescalerCount += ticks;
	auto realTicks = prescalerCount / PRESCALER_SELECTION[prescaler];
	prescalerCount = prescalerCount % PRESCALER_SELECTION[prescaler];
	return CounterUpdate(realTicks);
}

U32 Timer::Update(const U32& ticks, const U32& prevOverflow)
{
	U32 overflow = 0;

	if (timerStart) {
		if (countUp && !(ID == 0)) {
			overflow = CounterUpdate(prevOverflow);
		} else {
			overflow = PrescalerTimingUpdate(ticks);
		}
	}

	return overflow;
}