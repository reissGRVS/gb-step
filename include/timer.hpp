#pragma once

#include <array>

#include "memory.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

class Timer {
 public:
  Timer(std::shared_ptr<Memory> memory) : memory(memory){};

  void Update(std::uint32_t ticks) {
	std::uint32_t overflow = 0;

	for (auto timerIndex = 0u; timerIndex < 4u; timerIndex++) {
	  const std::uint32_t TMNCNT_L = TM0CNT_L + 0x4u * timerIndex;
	  const std::uint32_t TMNCNT_H = TM0CNT_H + 0x4u * timerIndex;
	  auto counter = memory->getHalf(TMNCNT_L);
	  auto timerCnt = memory->getHalf(TMNCNT_H);
	  auto prescaler = BIT_RANGE(timerCnt, 0, 1);
	  auto countUp = BIT_RANGE(timerCnt, 2, 2);
	  auto irqEnable = BIT_RANGE(timerCnt, 6, 6);
	  auto timerStart = BIT_RANGE(timerCnt, 7, 7);

	  if (timerStart) {
		if (countUp && !(timerIndex == 0)) {
		  if (overflow) {
			counter += overflow;
			if (counter != 0) {
			  overflow = 0;
			}
		  }
		} else {
		  prescalerCounts[timerIndex] += ticks;
		  if (prescalerCounts[timerIndex] >= PRESCALER_SELECTION[prescaler]) {
			prescalerCounts[timerIndex] = 0;
			counter++;
			overflow = (counter == 0) ? 1 : 0;
		  }
		}

		if (overflow) {
		  counter = reloadValues[timerIndex];
		  if (irqEnable) {
			spdlog::get("std")->info("Timer {:X} IntReq", timerIndex);
			auto intReq = memory->getHalf(IF);
			BIT_SET(intReq, 3 + timerIndex);
			memory->setHalf(IF, intReq);
		  }
		}

		memory->setHalf(TMNCNT_L, counter);
	  }
	}
  }

  void SetReloadValue(std::uint_fast8_t id, std::uint16_t value) {
	reloadValues[id] = value;
  }

  void TimerCntHUpdate(std::uint_fast8_t id, std::uint16_t value) {
	if (BIT_RANGE(timerCntH[id], 7, 7) == 0 && BIT_RANGE(value, 7, 7) == 1) {
	  memory->setHalf(TM0CNT_L + 0x4u * id, reloadValues[id]);
	}
	timerCntH[id] = value;
	memory->setHalf(TM0CNT_H + 0x4u * id, value);
  }

 private:
  std::shared_ptr<Memory> memory;
  std::array<std::uint32_t, 4> PRESCALER_SELECTION{1, 64, 256, 1024};

  std::array<std::uint16_t, 4> timerCntH{0, 0, 0, 0};
  std::array<std::uint32_t, 4> prescalerCounts{0, 0, 0, 0};
  std::array<std::uint32_t, 4> reloadValues{0, 0, 0, 0};
};