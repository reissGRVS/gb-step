#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"
#include "memory/regions.hpp"
#include <array>
#include <utility>

class IRIORegisters : public ReadWriteInterface {

public:
	static const U32 IR_IO_START = 0x04000200, IR_IO_END = 0x04000302;
	static const U32 IR_IO_SIZE = IR_IO_END - IR_IO_START + 1;

	enum Waitstate {
		WS0,
		WS1,
		WS2,
		SRAM,
		NUM_WAITSTATES
	};

	struct WaitstatePair {
		U8 nseq;
		U8 seq;
	};

	WaitstatePair GetWaitstateTicks(Waitstate ws)
	{
		return waitstateCounts[(U8)ws];
	}

protected:
	std::array<U8, IR_IO_SIZE> ioregisters{};

	bool interruptReady = false;
	void InterruptUpdate();

	bool halt = false;

	//Pairs of NSEQ and SEQ times
	std::array<WaitstatePair, NUM_WAITSTATES> waitstateCounts = {};
	void WaitstateControlUpdate();

private:
	const U32 IR_IE = IE - IR_IO_START;
	const U32 IR_IF = IF - IR_IO_START;
	const U32 IR_WAITCNT = WAITCNT - IR_IO_START;
	const U32 IR_IME = IME - IR_IO_START;
};