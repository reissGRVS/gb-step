#include "apu/apu.hpp"

	APU::APU(std::function<void()> FIFOACallback,std::function<void()> FIFOBCallback)
	: FifoCallbacks({FIFOACallback, FIFOBCallback})
	{
		audioStream.play();
	}


	const U32 TICK_THRESHOLD = 512;
	void APU::Tick(U32 t)
	{
		ticks += t;
		while (ticks > TICK_THRESHOLD)
		{
			ticks -= TICK_THRESHOLD;
			Sample();
		}
	}

	void APU::FIFOUpdate(U8 timerID)
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

	struct SoundCntH {
		SoundCntH(U16 v)
		{
			value = v;
			aVolume = BIT_RANGE(value, 2, 2) ? 4 : 2;
			bVolume = BIT_RANGE(value, 3, 3) ? 4 : 2;
			enableA[0] = BIT_RANGE(value, 8, 8);
			enableA[1] = BIT_RANGE(value, 9, 9);
			enableB[0] = BIT_RANGE(value, 12, 12);
			enableB[1] = BIT_RANGE(value, 13, 13);
		}
		U16 value;
		U8 aVolume;
		U8 bVolume;
		bool enableA[2];
		bool enableB[2];
	};

	void APU::Sample()
	{
		auto soundBias = Read(AccessSize::Half, SOUNDBIAS, Sequentiality::FREE);
		auto biasLevel = BIT_RANGE(soundBias, 1, 9);
		

		auto soundCntH = SoundCntH(Read(AccessSize::Half, SOUNDCNT_H, Sequentiality::FREE));

		for (U8 i = 0; i < 2; i++)
		{
			S16 sample = 0;
			if (soundCntH.enableA[i]) sample += lastSample[0] * soundCntH.aVolume;
			if (soundCntH.enableB[i]) sample += lastSample[1] * soundCntH.bVolume;
			sample += biasLevel;
			if (sample < 0) sample = 0;
			if (sample > 0x3FF) sample = 0x3FF;
			sample -= 0x200;
			sample *= 20;
			
			out.write(reinterpret_cast<char const *>(&sample), sizeof sample);
			audioStream.PushOne(sample);
		}
	}