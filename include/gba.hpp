#pragma once

#include <unistd.h>
#include "arm7tdmi/cpu.hpp"
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
  std::shared_ptr<Debugger> debugger;
  // TODO: Add interfaces for io here
};

class GBA {
 public:
  GBA(GBAConfig cfg)
      : memory(std::make_shared<Memory>(cfg.biosPath,
                                        cfg.romPath,
                                        cfg.joypad,
                                        cfg.debugger)),
        cpu(memory, std::move(cfg.debugger)),
        ppu(memory, cfg.screen){};

  void run() {
	while (true) {
	  auto ticks = cpu.Execute();
	  ppu.Execute(ticks);
	}
  };

 private:
  std::shared_ptr<Memory> memory;
  ARM7TDMI::CPU cpu;
  PPU ppu;
  Timer timer;
  DMA dma;
};