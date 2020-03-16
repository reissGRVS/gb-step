#pragma once

#include "int.hpp"
#include "apu/apu_io_registers.hpp"
#include "common/circular_queue.hpp"
#include "dma/controller.hpp"
#include <iostream>
#include "platform/sfml/audio.hpp"
#include <algorithm>
#include <fstream>

class APU : public APUIORegisters {

public:

	APU(std::function<void()> FIFOACallback,std::function<void()> FIFOBCallback)
	: FifoCallbacks({FIFOACallback, FIFOBCallback})
	{
		audioStream.play();
	}

	std::ofstream out{"gba_left.raw", std::ios::binary};

	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality&) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality&) override;


	U16 tickThreshold = 512;
	void Tick(U32 t)
	{
		ticks += t;
		while (ticks > tickThreshold)
		{
			ticks -= tickThreshold;
			Sample();
		}
	}

	void FIFOUpdate(U8 timerID)
	{
		for (auto i = 0; i <= 1; i++)
		{
			auto soundCntH = Read(AccessSize::Half, SOUNDCNT_H, Sequentiality::FREE);
			auto timerSelect = 10 + 4 *i;
			auto timer = BIT_RANGE(soundCntH, timerSelect, timerSelect);
			if (timer == timerID)
			{
				lastSample[i] = fifo[i].Pop();
				if (fifo[i].Size() <= FIFO_SIZE/2)
				{
					FifoCallbacks[i]();
				}
			}
		}
	}

	void Sample()
	{
		//todo: fix
		//auto soundBias = Read(AccessSize::Half, SOUNDBIAS, Sequentiality::FREE);
		//auto biasLevel = BIT_RANGE(soundBias, 1, 9);
		//auto amplitudeRes = BIT_RANGE(soundBias, 14, 15);
		auto soundCntH = Read(AccessSize::Half, SOUNDCNT_H, Sequentiality::FREE);
		auto soundAVolume = BIT_RANGE(soundCntH, 2, 2) ? 4 : 2;
		auto soundBVolume = BIT_RANGE(soundCntH, 3, 3) ? 4 : 2;
		auto soundAEnableRight = BIT_RANGE(soundCntH, 8, 8);
		auto soundAEnableLeft = BIT_RANGE(soundCntH, 9, 9);
		auto soundBEnableRight = BIT_RANGE(soundCntH, 12, 12);
		auto soundBEnableLeft = BIT_RANGE(soundCntH, 13, 13);

		S16 rightSample = 0;
		if (soundAEnableRight) rightSample += lastSample[0] * soundAVolume;
		if (soundBEnableRight) rightSample += lastSample[1] * soundBVolume;
		// rightSample += biasLevel;
		if (rightSample < 0) rightSample = 0;
		if (rightSample > 0x3FF) rightSample = 0x3FF;
		// rightSample -= 0x200;

		S16 leftSample = 0;
		if (soundAEnableLeft) leftSample += lastSample[0] * soundAVolume;
		if (soundBEnableLeft) leftSample += lastSample[1] * soundBVolume;
		// leftSample += biasLevel;
		if (leftSample < 0) leftSample = 0u;
		if (leftSample > 0x3FF) leftSample = 0x3FF;
		// leftSample -= 0x200;

		//audioStream.PushOne(rightSample);
		audioStream.PushOne(leftSample);
	}

private:
	std::array<std::function<void()>, 2> FifoCallbacks;
	static const U8 FIFO_SIZE = 32;
	CircularQueue<S8, FIFO_SIZE> fifo[2];
	S8 lastSample[2] = {};
	U32 ticks = 0;
	AudioStream audioStream;

};
