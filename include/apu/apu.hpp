#pragma once

#include "apu/apu_io_registers.hpp"
#include "common/circular_queue.hpp"
#include "dma/controller.hpp"
#include "int.hpp"
#include "platform/sfml/audio.hpp"
#include <algorithm>
#include <fstream>

class APU : public APUIORegisters {

public:
	APU(std::function<void()> FIFOACallback, std::function<void()> FIFOBCallback);

	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality&) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality&) override;

	void Tick(U32 t);
	void FIFOUpdate(U8 timerID);

	void Sample();

private:
	std::ofstream out { "gba.raw", std::ios::binary };

	std::array<std::function<void()>, 2> FifoCallbacks;

	static const U8 FIFO_SIZE = 32;
	CircularQueue<S8, FIFO_SIZE> fifo[2];

	S8 lastSample[2] = {};

	U32 ticks = 0;

	AudioStream audioStream;
};
