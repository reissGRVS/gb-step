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
			memory(std::make_unique<Memory>(cfg.biosPath, cfg.romPath))
		{};
		void Run() {};

	private:
		std::unique_ptr<Memory> memory;
		CPU cpu;
		PPU ppu;
		Timer timer;
		DMA dma;
};

