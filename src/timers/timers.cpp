#include "timers/timers.hpp"

Timers::Timers(std::shared_ptr<IRQChannel> irqChannel, std::function<void(U8)> apuCallback)
	: irqChannel(irqChannel), apuCallback(apuCallback)
{
}

void Timers::Update(const U32 ticks)
{
	heldTicks += ticks;
	
	if (!ticksToOverflowKnown)
	{
		U32 curMinTicks = 0xFFFFFFFF;
		for (auto& timer : timers)
		{
			if (timer.ticksLeft < curMinTicks)
			{
				curMinTicks = timer.ticksLeft;
			}
		}
		minTicks = curMinTicks;
		ticksToOverflowKnown = true;
	}

	if (heldTicks >= minTicks)
	{
		U32 overflow = 0;
		for (auto timerIndex = 0u; timerIndex < 4u; timerIndex++) {
			overflow = timers[timerIndex].Update(heldTicks, overflow);

			if (overflow && timers[timerIndex].irqEnable) {
				
				irqChannel->RequestInterrupt(timerInterrupts[timerIndex]);
			}

			if (overflow && (timerIndex == 0 || timerIndex == 1))
			{
				apuCallback(timerIndex);
			}
		}
		ticksToOverflowKnown = false;
		heldTicks = 0;
	}
}

void Timers::SetReloadValue(U8 id, U16 value)
{
	timers[id].reloadValue = value;
}

void Timers::TimerCntHUpdate(U8 id, U16 value)
{
	auto oldStart = timers[id].timerStart;
	timers[id].UpdateCntH(value);

	if (oldStart == 0 && timers[id].timerStart == 1) {
		timers[id].prescalerCount = 0;
	}
	ticksToOverflowKnown = false;
}