#pragma once

#include "memory.hpp"
#include "screen.hpp"

class PPU {
  enum State { Visible, HBlank, VBlank };

 public:
  PPU(std::shared_ptr<Memory> memory_, Screen& screen_)
      : memory(memory_), screen(screen_) {}
  void Execute(std::uint32_t ticks);

 private:
  void fetchScanline();

  std::uint16_t getBgColorFromPallete(const std::uint32_t& colorID);
  std::uint8_t getByte(const std::uint32_t& address);
  std::uint16_t getHalf(const std::uint32_t& address);
  void setHalf(const std::uint32_t& address, const std::uint16_t& value);

  std::shared_ptr<Memory> memory;
  Screen& screen;
  Screen::Framebuffer fb{};
  State state = Visible;
  std::uint32_t tickCount;

  const std::uint32_t CYCLES_PER_VISIBLE = 960, CYCLES_PER_HBLANK = 272,
                      CYCLES_PER_LINE = CYCLES_PER_VISIBLE + CYCLES_PER_HBLANK,
                      VISIBLE_LINES = 160, VBLANK_LINES = 68,
                      TOTAL_LINES = VISIBLE_LINES + VBLANK_LINES,
                      CYCLES_PER_VBLANK = CYCLES_PER_LINE * VBLANK_LINES;
};
