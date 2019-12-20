#pragma once

#include <unistd.h>
#include <functional>
#include "arm7tdmi/cpu.hpp"
#include "debugger.hpp"
#include "dma_controller.hpp"
#include "memory.hpp"
#include "ppu/ppu.hpp"
#include "screen.hpp"
#include "spdlog/spdlog.h"
#include "system_clock.hpp"
#include "timers.hpp"

struct GBAConfig {
  std::string biosPath;
  std::string romPath;
  Screen& screen;
  Joypad& joypad;
  // TODO: Add interfaces for io here
};

class GBA {
 public:
  GBA(GBAConfig cfg)
      : sysClock(std::make_shared<SystemClock>()),
        memory(std::make_shared<Memory>(sysClock,
                                        cfg.biosPath,
                                        cfg.romPath,
                                        cfg.joypad)),
        cpu(sysClock, memory),
        ppu(memory, cfg.screen),
        debugger(memory),
        timers(memory),
        dma(memory) {
	memory->SetDebugWriteCallback(std::bind(&Debugger::NotifyMemoryWrite,
	                                        &debugger, std::placeholders::_1));

	memory->SetIOWriteCallback(IF, [&](std::uint32_t irAcknowledge) {
	  auto currentIF = memory->getHalf(IF);
	  spdlog::get("std")->debug("IRQ Acknowledge {:B} was {:X}", irAcknowledge,
	                            currentIF);
	  currentIF &= ~irAcknowledge;

	  memory->setHalf(IF, currentIF);
	  spdlog::get("std")->debug("IRQ Acknowledge now {:X}", currentIF);
	});

	memory->SetIOWriteCallback(
	    DMA0CNT_H, std::bind(&DMAController::CntHUpdateCallback, &dma, 0,
	                         std::placeholders::_1));
	memory->SetIOWriteCallback(
	    DMA1CNT_H, std::bind(&DMAController::CntHUpdateCallback, &dma, 1,
	                         std::placeholders::_1));
	memory->SetIOWriteCallback(
	    DMA2CNT_H, std::bind(&DMAController::CntHUpdateCallback, &dma, 2,
	                         std::placeholders::_1));
	memory->SetIOWriteCallback(
	    DMA3CNT_H, std::bind(&DMAController::CntHUpdateCallback, &dma, 3,
	                         std::placeholders::_1));

	memory->SetIOWriteCallback(
	    TM0CNT_L,
	    std::bind(&Timers::SetReloadValue, &timers, 0, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM0CNT_H,
	    std::bind(&Timers::TimerCntHUpdate, &timers, 0, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM1CNT_L,
	    std::bind(&Timers::SetReloadValue, &timers, 1, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM1CNT_H,
	    std::bind(&Timers::TimerCntHUpdate, &timers, 1, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM2CNT_L,
	    std::bind(&Timers::SetReloadValue, &timers, 2, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM2CNT_H,
	    std::bind(&Timers::TimerCntHUpdate, &timers, 2, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM3CNT_L,
	    std::bind(&Timers::SetReloadValue, &timers, 3, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM3CNT_H,
	    std::bind(&Timers::TimerCntHUpdate, &timers, 3, std::placeholders::_1));
  };

  void run() {
	while (true) {
#ifndef NDEBUG
	  debugger.CheckForBreakpoint(cpu.ViewState());
#endif
	  cpu.Execute();
	  auto ticks = sysClock->SinceLastCheck();
	  ppu.Execute(ticks);
	  timers.Update(ticks);
	}
  };

 private:
  std::shared_ptr<SystemClock> sysClock;
  std::shared_ptr<Memory> memory;
  ARM7TDMI::CPU cpu;
  PPU ppu;
  Debugger debugger;
  Timers timers;
  DMAController dma;
};
