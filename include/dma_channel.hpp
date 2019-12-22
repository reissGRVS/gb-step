#pragma once
#include "memory.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

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
	auto prevEnable = enable;
	memory->setHalf(CNT_H, value);
	dmaCnt = value;
	enable = BIT_RANGE(dmaCnt, 15, 15);

	// Set internal registers on DMA enable
	if (enable == 1 && prevEnable == 0) {
	  if (ID == 0) {
		source = memory->getWord(SAD) & NBIT_MASK(27);
	  } else {
		source = memory->getWord(SAD) & NBIT_MASK(28);
	  }

	  if (ID == 3) {
		dest = memory->getWord(DAD) & NBIT_MASK(28);
	  } else {
		dest = memory->getWord(DAD) & NBIT_MASK(27);
	  }

	  if (ID == 3) {
		wordCount = memory->getHalf(CNT_L) & NBIT_MASK(16);
	  } else {
		wordCount = memory->getHalf(CNT_L) & NBIT_MASK(14);
	  }

	  if (wordCount == 0) {
		wordCount = (ID == 3) ? 0x10000 : 0x4000;
	  }
	  spdlog::get("std")->debug("DMA Set Internal Registers");
	}

	repeat = BIT_RANGE(dmaCnt, 9, 9);
	startTiming = BIT_RANGE(dmaCnt, 12, 13);
	irqAtEnd = BIT_RANGE(dmaCnt, 14, 14);

	spdlog::get("std")->debug(
	    "DMA Details Ch {}: enable {}, startTiming {}, src {:X}, dst {:X}, "
	    "irqAtEnd {}, repeat {}",
	    ID, enable, startTiming, source, dest, irqAtEnd, repeat);
  }

  void doTransfer() {
	auto transferType = BIT_RANGE(dmaCnt, 10, 10);
	auto transferSize = transferType ? 4 : 2;
	auto destAddrCtl = BIT_RANGE(dmaCnt, 5, 6);
	auto srcAddrCtl = BIT_RANGE(dmaCnt, 7, 8);
	auto firstSrcVal = memory->getWord(source);
	spdlog::get("std")->debug(
	    "DMA{} starting: src {:X}, dst {:X}, wordCount {:X}, transferType "
	    "{}, "
	    "destCtl {}, srcCtl {}, srcVal {:X}",
	    ID, source, dest, wordCount, transferType, destAddrCtl, srcAddrCtl,
	    firstSrcVal);

	auto seq = NSEQ;
	while (wordCount > 0) {
	  // 0 -> 16bit  1 -> 32bit
	  if (transferType) {
		memory->Write(Word, dest, memory->Read(Word, source, seq), seq);
	  } else {
		memory->Write(Word, dest, memory->Read(Word, source, seq), seq);
	  }
	  seq = SEQ;
	  // Update internal registers
	  {
		// Dest Addr Control
		switch (destAddrCtl) {
		  case 0: {
			dest += transferSize;
			break;
		  }
		  case 1: {
			dest -= transferSize;
			break;
		  }
		  default:
			break;
		}

		// Src Addr Control
		switch (srcAddrCtl) {
		  case 0: {
			source += transferSize;
			break;
		  }
		  case 1: {
			source -= transferSize;
			break;
		  }
		  default:
			break;
		}

		wordCount--;
	  }
	}

	spdlog::get("std")->debug(
	    "DMA{} finishing: src {:X}, dst {:X}, wordCount {:X}, transferType "
	    "{}, "
	    "destCtl {}, srcCtl {}",
	    ID, source, dest, wordCount, transferType, destAddrCtl, srcAddrCtl);
	if (repeat) {
	  wordCount = memory->getHalf(CNT_L) & NBIT_MASK(14);
	  if (wordCount == 0) {
		wordCount = 0x4000;
	  }
	} else {
	  BIT_CLEAR(dmaCnt, 15);
	  enable = 0;
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
};