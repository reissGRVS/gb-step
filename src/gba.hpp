#pragma once

#include "arm7tdmi/cpu.hpp"
#include "ppu.hpp"
#include "memory.hpp"
#include "timer.hpp"
#include "dma.hpp"

struct GBAConfig
{
	std::string biosPath;
	std::string romPath;
	//TODO: Add interfaces for io here
};

class GBA
{
	public:
		GBA(GBAConfig cfg) :
			memory(std::make_shared<Memory>(cfg.biosPath, cfg.romPath)),
			cpu(memory)
		{};
		
		void run() {};

	private: 
		std::shared_ptr<Memory> memory;
		ARM7TDMI::CPU cpu;
		PPU ppu;
		Timer timer;
		DMA dma;
};

