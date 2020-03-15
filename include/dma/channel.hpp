#pragma once
#include "memory/memory.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace DMA {
class Channel {
public:
	Channel(std::int_fast8_t id, std::shared_ptr<Memory> memory_);

	void ReloadDAD();
	void ReloadSAD();
	void ReloadWordCount();
	void CalculateTransferSteps();
	void UpdateDetails(U16 value);

	void DoTransferStep();
	void DoSoundTransfer();

	const std::int_fast8_t ID;
	const U32 SAD;
	const U32 DAD;
	const U32 CNT_L;
	const U32 CNT_H;

	bool active = false;

	U16 dmaCnt;
	U16 enable = 0;
	U16 repeat;
	U16 startTiming;
	U16 irqAtEnd;
	U16 transferType;
	U16 transferSize;
	U16 srcStep;
	U16 destStep;
	U16 destAddrCtl;
	U16 srcAddrCtl;

	U32 dest;

private:
	std::shared_ptr<Memory> memory;

	U32 source;
	U32 wordCount;
};
} // namespace DMA