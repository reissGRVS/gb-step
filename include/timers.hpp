#pragma once

#include <array>

#include "memory/memory.hpp"
#include "spdlog/spdlog.h"
#include "timer.hpp"
#include "utils.hpp"

class Timers {
public:
	Timers(std::shared_ptr<Memory> memory)
		: memory(memory){};

	void Update(const std::uint32_t ticks)
	{
		std::uint32_t overflow = 0;
		for (auto timerIndex = 0u; timerIndex < 4u; timerIndex++) {
			overflow = timers[timerIndex].Update(ticks, overflow);

			if (overflow && timers[timerIndex].irqEnable) {
				spdlog::get("std")->debug("Timer {:X} IntReq", timerIndex);
				memory->RequestInterrupt(timerInterrupts[timerIndex]);
			}
		}
	}

	void SetReloadValue(std::uint_fast8_t id, std::uint16_t value)
	{
		timers[id].reloadValue = value;
	}

	void TimerCntHUpdate(std::uint_fast8_t id, std::uint16_t value)
	{
		if (BIT_RANGE(timers[id].cntHData, 7, 7) == 0 && BIT_RANGE(value, 7, 7) == 1) {
			memory->SetHalf(timers[id].CNT_L, timers[id].reloadValue);
			timers[id].prescalerCount = 0;
		}
		timers[id].UpdateCntH(value);
		memory->SetHalf(timers[id].CNT_H, value);
	}

private:
	std::shared_ptr<Memory> memory;
	const std::array<Interrupt, 4> timerInterrupts{
		Interrupt::Timer0, Interrupt::Timer1, Interrupt::Timer2,
		Interrupt::Timer3
	};
	std::array<Timer, 4> timers{
		Timer(0, TM0CNT_L, memory), Timer(1, TM1CNT_L, memory),
		Timer(2, TM2CNT_L, memory), Timer(3, TM3CNT_L, memory)
	};
};