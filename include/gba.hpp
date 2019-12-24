#pragma once

#include "arm7tdmi/cpu.hpp"
#include "debugger.hpp"
#include "dma/controller.hpp"
#include "memory/memory.hpp"
#include "ppu/ppu.hpp"
#include "screen.hpp"
#include "spdlog/spdlog.h"
#include "system_clock.hpp"
#include "timers/timers.hpp"
#include <functional>
#include <unistd.h>

struct GBAConfig {
	std::string biosPath;
	std::string romPath;
	Screen& screen;
	Joypad& joypad;
};

class GBA {
public:
	GBA(GBAConfig cfg)
		: sysClock(std::make_shared<SystemClock>())
		, memory(std::make_shared<Memory>(sysClock,
			  cfg.biosPath,
			  cfg.romPath,
			  cfg.joypad))
		, cpu(std::make_shared<ARM7TDMI::CPU>(sysClock, memory))
		, ppu(std::make_shared<PPU>(memory, cfg.screen))
		, debugger(memory)
		, timers(std::make_shared<Timers>(memory))
		, dma(std::make_shared<DMA::Controller>(memory))
	{
		memory->SetDebugWriteCallback(std::bind(&Debugger::NotifyMemoryWrite,
			&debugger, std::placeholders::_1));

		auto ioRegisters = std::make_shared<IORegisters>(
			std::static_pointer_cast<TimersIORegisters>(timers),
			std::static_pointer_cast<DMAIORegisters>(dma),
			std::static_pointer_cast<LCDIORegisters>(ppu),
			std::static_pointer_cast<IRIORegisters>(cpu));
		memory->AttachIORegisters(ioRegisters);
		memory->AttachIRIORegisters(std::static_pointer_cast<IRIORegisters>(cpu));
		cpu->Reset();

		ppu->HBlankCallback
			= std::bind(&DMA::Controller::EventCallback, dma,
				DMA::Controller::Event::HBLANK, std::placeholders::_1);
		ppu->VBlankCallback = std::bind(&DMA::Controller::EventCallback, dma,
			DMA::Controller::Event::VBLANK, std::placeholders::_1);

		// memory->SetIOWriteCallback(
		// 	DMA0CNT_H, std::bind(&DMA::Controller::CntHUpdateCallback, &dma, 0, std::placeholders::_1));
		// memory->SetIOWriteCallback(
		// 	DMA1CNT_H, std::bind(&DMA::Controller::CntHUpdateCallback, &dma, 1, std::placeholders::_1));
		// memory->SetIOWriteCallback(
		// 	DMA2CNT_H, std::bind(&DMA::Controller::CntHUpdateCallback, &dma, 2, std::placeholders::_1));
		// memory->SetIOWriteCallback(
		// 	DMA3CNT_H, std::bind(&DMA::Controller::CntHUpdateCallback, &dma, 3, std::placeholders::_1));
	};

	void run()
	{
		while (true) {
#ifndef NDEBUG
			debugger.CheckForBreakpoint(cpu->ViewState());
#endif
			if (dma->IsActive()) {
				dma->Execute();
			} else {
				cpu->Execute();
			}

			auto ticks = sysClock->SinceLastCheck();
			ppu->Execute(ticks);
			timers->Update(ticks);
		}
	};

private:
	std::shared_ptr<SystemClock> sysClock;
	std::shared_ptr<Memory> memory;
	std::shared_ptr<ARM7TDMI::CPU> cpu;
	std::shared_ptr<PPU> ppu;
	Debugger debugger;
	std::shared_ptr<Timers> timers;
	std::shared_ptr<DMA::Controller> dma;
};
