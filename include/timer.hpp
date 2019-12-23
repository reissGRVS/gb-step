#include "utils.hpp"

struct Timer {
	Timer(std::uint8_t ID, std::uint32_t CNT_L, std::shared_ptr<Memory> memory)
		: ID(ID)
		, CNT_L(CNT_L)
		, CNT_H(CNT_L + 2)
		, memory(memory)
	{
	}

	void UpdateCntH(std::uint16_t value)
	{
		cntHData = value;
		prescaler = BIT_RANGE(cntHData, 0, 1);
		countUp = BIT_RANGE(cntHData, 2, 2);
		irqEnable = BIT_RANGE(cntHData, 6, 6);
		timerStart = BIT_RANGE(cntHData, 7, 7);
	}

	std::uint32_t CounterUpdate(std::uint32_t ticks)
	{
		std::uint32_t overflow = 0;

		auto ticksLeft = 0x10000u - counter;

		if (ticksLeft <= ticks) {
			ticks -= ticksLeft;
			auto tickRange = 0x10000 - reloadValue;
			overflow += 1 + (ticks / tickRange);
			counter = reloadValue + (ticks % tickRange);
		} else {
			counter += ticks;
		}

		return overflow;
	}

	std::uint32_t PrescalerTimingUpdate(std::uint32_t ticks)
	{
		prescalerCount += ticks;
		auto realTicks = prescalerCount / PRESCALER_SELECTION[prescaler];
		prescalerCount = prescalerCount % PRESCALER_SELECTION[prescaler];
		return CounterUpdate(realTicks);
	}

	std::uint32_t Update(std::uint32_t ticks, std::uint32_t prevOverflow)
	{
		counter = memory->GetHalf(CNT_L);

		std::uint32_t overflow = 0;

		if (timerStart) {
			if (countUp && !(ID == 0)) {
				overflow = CounterUpdate(prevOverflow);
			} else {
				overflow = PrescalerTimingUpdate(ticks);
			}
			memory->SetHalf(CNT_L, counter);
		}

		return overflow;
	};

	const std::uint8_t ID;
	const std::uint32_t CNT_L;
	const std::uint32_t CNT_H;

	std::uint16_t cntHData;
	std::uint16_t prescaler;
	std::uint16_t countUp;
	std::uint16_t irqEnable;
	std::uint16_t timerStart;

	std::uint16_t counter;
	std::uint16_t reloadValue;
	std::uint32_t prescalerCount;

	const std::array<std::uint16_t, 4> PRESCALER_SELECTION{ 1, 64, 256, 1024 };

	std::shared_ptr<Memory> memory;
};