#include "dma_channel.hpp"

DMAChannel::DMAChannel(std::int_fast8_t id, std::shared_ptr<Memory> memory_)
    : ID(id),
      SAD(DMA0SAD + ID * 0xC),
      DAD(DMA0DAD + ID * 0xC),
      CNT_L(DMA0CNT_L + ID * 0xC),
      CNT_H(DMA0CNT_H + ID * 0xC),
      memory(memory_) {}

void DMAChannel::ReloadDAD() {
  if (ID == 3) {
	dest = memory->getWord(DAD) & NBIT_MASK(28);
  } else {
	dest = memory->getWord(DAD) & NBIT_MASK(27);
  }
}

void DMAChannel::ReloadSAD() {
  if (ID == 0) {
	source = memory->getWord(SAD) & NBIT_MASK(27);
  } else {
	source = memory->getWord(SAD) & NBIT_MASK(28);
  }
}

void DMAChannel::ReloadWordCount() {
  if (ID == 3) {
	wordCount = memory->getHalf(CNT_L) & NBIT_MASK(16);
  } else {
	wordCount = memory->getHalf(CNT_L) & NBIT_MASK(14);
  }
  if (wordCount == 0) {
	wordCount = (ID == 3) ? 0x10000 : 0x4000;
  }
}

void DMAChannel::CalculateTransferSteps() {
  switch (destAddrCtl) {
	case 0:
	case 3: {
	  destStep = transferSize;
	  break;
	}
	case 1: {
	  destStep = -transferSize;
	  break;
	}
	default: {
	  destStep = 0;
	  break;
	}
  }
  switch (srcAddrCtl) {
	case 0:
	case 3: {
	  srcStep = transferSize;
	  break;
	}
	case 1: {
	  srcStep = -transferSize;
	  break;
	}
	default: {
	  srcStep = 0;
	  break;
	}
  }
}

void DMAChannel::UpdateDetails(std::uint16_t value) {
  auto prevEnable = enable;
  memory->setHalf(CNT_H, value);
  dmaCnt = value;
  enable = BIT_RANGE(dmaCnt, 15, 15);

  // Set internal registers on DMA enable
  if (enable == 1 && prevEnable == 0) {
	ReloadSAD();
	ReloadDAD();
	ReloadWordCount();
	spdlog::get("std")->debug("DMA Set Internal Registers");
  } else if (enable == 0) {
	active = false;
  }

  repeat = BIT_RANGE(dmaCnt, 9, 9);
  startTiming = BIT_RANGE(dmaCnt, 12, 13);
  irqAtEnd = BIT_RANGE(dmaCnt, 14, 14);
  transferType = BIT_RANGE(dmaCnt, 10, 10);
  transferSize = transferType ? 4 : 2;
  destAddrCtl = BIT_RANGE(dmaCnt, 5, 6);
  srcAddrCtl = BIT_RANGE(dmaCnt, 7, 8);
  CalculateTransferSteps();

  spdlog::get("std")->debug(
      "DMA Details Ch {}: enable {}, startTiming {}, src {:X}, dst {:X}, "
      "irqAtEnd {}, repeat {}, destStp {:X}, srcStp {:X}",
      ID, enable, startTiming, source, dest, irqAtEnd, repeat, destStep,
      srcStep);
}

void DMAChannel::DoTransferStep() {
  if (wordCount > 0) {
	// TODO: if first recent transfer NSEQ
	if (transferType) {
	  memory->Write(Word, dest, memory->Read(Word, source, SEQ), SEQ);
	} else {
	  memory->Write(Word, dest, memory->Read(Word, source, SEQ), SEQ);
	}
	dest += destStep;
	source += srcStep;
	wordCount--;
  } else {
	if (repeat) {
	  ReloadWordCount();
	  if (destAddrCtl == 3) {
		ReloadDAD();
	  }
	} else {
	  // Transfer Finished
	  BIT_CLEAR(dmaCnt, 15);
	  memory->setHalf(CNT_H, dmaCnt);
	  enable = 0;
	  active = false;
	}
  }
}