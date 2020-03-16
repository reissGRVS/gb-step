#include "dma/channel.hpp"

using namespace DMA;
Channel::Channel(std::int_fast8_t id, std::shared_ptr<Memory> memory_)
	: ID(id)
	, SAD(DMA0SAD + ID * 0xC)
	, DAD(DMA0DAD + ID * 0xC)
	, CNT_L(DMA0CNT_L + ID * 0xC)
	, CNT_H(DMA0CNT_H + ID * 0xC)
	, memory(memory_)
{
}

void Channel::ReloadDAD()
{
	if (ID == 3) {
		dest = memory->GetWord(DAD) & NBIT_MASK(28);
	} else {
		dest = memory->GetWord(DAD) & NBIT_MASK(27);
	}
}

void Channel::ReloadSAD()
{
	if (ID == 0) {
		source = memory->GetWord(SAD) & NBIT_MASK(27);
	} else {
		source = memory->GetWord(SAD) & NBIT_MASK(28);
	}
}

void Channel::ReloadWordCount()
{
	if (ID == 3) {
		wordCount = memory->GetHalf(CNT_L) & NBIT_MASK(16);
	} else {
		wordCount = memory->GetHalf(CNT_L) & NBIT_MASK(14);
	}
	if (wordCount == 0) {
		wordCount = (ID == 3) ? 0x10000 : 0x4000;
	}
}

void Channel::CalculateTransferSteps()
{
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

void Channel::UpdateDetails(U16 value)
{
	auto prevEnable = enable;
	dmaCnt = value;
	enable = BIT_RANGE(dmaCnt, 15, 15);

	// Set internal registers on DMA enable
	if (enable == 1 && prevEnable == 0) {
		ReloadSAD();
		ReloadDAD();
		ReloadWordCount();
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

}

void Channel::DoSoundTransfer(){

	for (int i = 0; i < 4; i++)
	{
		U32 readVal = memory->Read(Word, source, SEQ);
		memory->Write(Word, dest, readVal, SEQ);
		source += srcStep;
	}
	active = false;
}

void Channel::DoTransferStep()
{
	if (wordCount > 0) {
		// TODO: if first recent transfer NSEQ
		if (transferType) {
			memory->Write(Word, dest, memory->Read(Word, source, SEQ), SEQ);
		} else {
			memory->Write(Half, dest, memory->Read(Half, source, SEQ), SEQ);
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
			memory->SetHalf(CNT_H, dmaCnt);
			enable = 0;
			active = false;
		}
	}
}
