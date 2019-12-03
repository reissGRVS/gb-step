#pragma once
#include "memory.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

// TODO Extend this to work for all channels
class DMAChannel {
 public:
  DMAChannel(std::int_fast8_t id, std::shared_ptr<Memory> memory_)
      : ID(id),
        SAD(DMA0SAD + ID * 0xC),
        DAD(DMA0DAD + ID * 0xC),
        CNT_L(DMA0CNT_L + ID * 0xC),
        CNT_H(DMA0CNT_H + ID * 0xC),
        memory(memory_){};

  void updateDetails(std::uint16_t value) {
	spdlog::get("std")->info("DMA Details {}", ID);
	auto prevEnable = enable;
	memory->setHalf(CNT_H, value);
	dmaCnt = value;
	enable = BIT_RANGE(dmaCnt, 15, 15);

	// Set internal registers on DMA enable
	if (enable == 1 && prevEnable == 0) {
	  source = memory->getWord(SAD) & NBIT_MASK(27);
	  // TODO: 28 and 16 and 0x10000 for Id3
	  dest = memory->getWord(DAD) & NBIT_MASK(27);
	  wordCount = memory->getHalf(CNT_L) & NBIT_MASK(14);
	  if (wordCount == 0) {
		wordCount = 0x4000;
	  }
	}

	repeat = BIT_RANGE(dmaCnt, 7, 8);
	startTiming = BIT_RANGE(dmaCnt, 12, 13);
	irqAtEnd = BIT_RANGE(dmaCnt, 14, 14);
  }

  void doTransfer() {
	spdlog::get("std")->info("DMA {}", ID);
	auto transferType = BIT_RANGE(dmaCnt, 10, 10);

	while (wordCount > 0) {
	  // 0 -> 16bit  1 -> 32bit
	  if (transferType) {
		memory->setHalf(dest, memory->getHalf(source));
	  } else {
		memory->setWord(dest, memory->getWord(source));
	  }

	  // Update internal registers
	  {
		// Dest Addr Control
		switch (BIT_RANGE(dmaCnt, 5, 6)) {
		  case 0: {
			dest++;
			break;
		  }
		  case 1: {
			dest--;
			break;
		  }
		  default:
			break;
		}

		// Dest Addr Control
		switch (BIT_RANGE(dmaCnt, 7, 8)) {
		  case 0: {
			source++;
			break;
		  }
		  case 1: {
			source--;
			break;
		  }
		  default:
			break;
		}

		wordCount--;
	  }
	}

	if (repeat) {
	  wordCount = memory->getHalf(CNT_L) & NBIT_MASK(14);
	  if (wordCount == 0) {
		wordCount = 0x4000;
	  }
	} else {
	  BIT_CLEAR(dmaCnt, 15);
	}
  }

  const std::int_fast8_t ID;
  const std::uint32_t SAD;
  const std::uint32_t DAD;
  const std::uint32_t CNT_L;
  const std::uint32_t CNT_H;

  std::uint16_t dmaCnt;
  std::uint16_t enable;
  std::uint16_t repeat;
  std::uint16_t startTiming;
  std::uint16_t irqAtEnd;

  std::uint32_t source;
  std::uint32_t dest;
  std::uint16_t wordCount;

 private:
  std::shared_ptr<Memory> memory;
  // TODO: Change memory seq
};