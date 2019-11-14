#pragma once

#include <unistd.h>
#include <functional>
#include "arm7tdmi/cpu.hpp"
#include "debugger.hpp"
#include "dma.hpp"
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
        debugger(memory) {
	memory->SetPublishWriteCallback(std::bind(
	    &Debugger::NotifyMemoryWrite, &debugger, std::placeholders::_1));
  };

  void run() {
	while (true) {
#ifndef NDEBUG
	  debugger.CheckForBreakpoint(cpu.ViewState());
#endif
	  auto ticks = cpu.Execute();
	  ppu.Execute(ticks);
	}
  };

 private:
  std::shared_ptr<Memory> memory;
  ARM7TDMI::CPU cpu;
  PPU ppu;
  Debugger debugger;
  Timer timer;
  DMA dma;
};
