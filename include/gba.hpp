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
  Screen &screen;
  Joypad &joypad;
};

class GBA {
public:
  GBA(GBAConfig cfg)
      : cfg(cfg), sysClock(std::make_shared<SystemClock>()),
        memory(std::make_shared<Memory>(sysClock, cfg.biosPath, cfg.romPath,
                                        cfg.joypad)),
        cpu(std::make_shared<ARM7TDMI::CPU>(sysClock, memory)),
        dma(std::make_shared<DMA::Controller>(memory)),
        ppu(std::make_shared<PPU>(
            memory, cfg.screen,
            std::bind(&DMA::Controller::EventCallback, dma,
                      DMA::Controller::Event::HBLANK, std::placeholders::_1),
            std::bind(&DMA::Controller::EventCallback, dma,
                      DMA::Controller::Event::VBLANK, std::placeholders::_1))),
        debugger(memory), timers(std::make_shared<Timers>(memory)) {
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
  };

  ~GBA(){memory->Save();}
  
  void run() {
    while (!cfg.joypad.esc) {
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
  GBAConfig cfg;
  std::shared_ptr<SystemClock> sysClock;
  std::shared_ptr<Memory> memory;
  std::shared_ptr<ARM7TDMI::CPU> cpu;
  std::shared_ptr<DMA::Controller> dma;
  std::shared_ptr<PPU> ppu;
  Debugger debugger;
  std::shared_ptr<Timers> timers;
};
