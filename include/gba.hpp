#pragma once

#include <unistd.h>
#include <functional>
#include "arm7tdmi/cpu.hpp"
#include "debugger.hpp"
#include "dma_controller.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "screen.hpp"
#include "timer.hpp"

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
      : memory(std::make_shared<Memory>(cfg.biosPath, cfg.romPath, cfg.joypad)),
        cpu(memory),
        ppu(memory, cfg.screen),
        debugger(memory),
        timer(memory),
        dma(memory) {
	memory->SetDebugWriteCallback(std::bind(&Debugger::NotifyMemoryWrite,
	                                        &debugger, std::placeholders::_1));
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
	    std::bind(&Timer::SetReloadValue, &timer, 0, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM0CNT_H,
	    std::bind(&Timer::TimerCntHUpdate, &timer, 0, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM1CNT_L,
	    std::bind(&Timer::SetReloadValue, &timer, 1, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM1CNT_H,
	    std::bind(&Timer::TimerCntHUpdate, &timer, 1, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM2CNT_L,
	    std::bind(&Timer::SetReloadValue, &timer, 2, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM2CNT_H,
	    std::bind(&Timer::TimerCntHUpdate, &timer, 2, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM3CNT_L,
	    std::bind(&Timer::SetReloadValue, &timer, 3, std::placeholders::_1));
	memory->SetIOWriteCallback(
	    TM3CNT_H,
	    std::bind(&Timer::TimerCntHUpdate, &timer, 3, std::placeholders::_1));
  };

  void run() {
	while (true) {
#ifndef NDEBUG
	  debugger.CheckForBreakpoint(cpu.ViewState());
#endif
	  auto ticks = cpu.Execute();
	  ppu.Execute(ticks);
	  timer.Update(ticks);
	}
  };

 private:
  std::shared_ptr<Memory> memory;
  ARM7TDMI::CPU cpu;
  PPU ppu;
  Debugger debugger;
  Timer timer;
  DMAController dma;
};
