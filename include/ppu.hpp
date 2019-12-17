#pragma once

#include "memory.hpp"
#include "screen.hpp"
#include "utils.hpp"

const std::uint32_t BG_TILE_DATA_UNITSIZE = 0x4000u;
const std::uint32_t BG_MAP_DATA_UNITSIZE = 0x800u;
const std::uint32_t BGCNT[4] = {BG0CNT, BG1CNT, BG2CNT, BG3CNT};
struct BGControlInfo {
  BGControlInfo(const std::uint_fast8_t& bgID, const uint32_t& bgCnt)
      : ID(bgID),
        priority(BIT_RANGE(bgCnt, 0, 1)),
        tileDataBase(VRAM_START +
                     BIT_RANGE(bgCnt, 2, 3) * BG_TILE_DATA_UNITSIZE),
        mosaic(BIT_RANGE(bgCnt, 6, 6)),
        colorsPalettes(BIT_RANGE(bgCnt, 7, 7)),
        colorDepth(colorsPalettes ? 8u : 4u),
        pixelsPerByte(8 / colorDepth),
        mapDataBase(VRAM_START +
                    BIT_RANGE(bgCnt, 8, 12) * BG_MAP_DATA_UNITSIZE),
        wrapAround(BIT_RANGE(bgCnt, 13, 13) || ID == 0 || ID == 1),
        screenSize(BIT_RANGE(bgCnt, 14, 15)){};

  const std::uint_fast8_t ID;

  const std::uint_fast8_t priority;
  const std::uint32_t tileDataBase;
  const bool mosaic;
  const uint_fast8_t colorsPalettes;
  const uint_fast8_t colorDepth;
  const uint_fast8_t pixelsPerByte;

  const std::uint32_t mapDataBase;
  const bool wrapAround;
  const uint_fast8_t screenSize;
};

class PPU {
  enum State { Visible, HBlank, VBlank };

 public:
  PPU(std::shared_ptr<Memory> memory_, Screen& screen_)
      : memory(memory_), screen(screen_) {}
  void Execute(std::uint32_t ticks);

 private:
  void drawLine();

  std::uint16_t TilePixelAtAbsoluteBGPosition(const BGControlInfo& bgCnt,
                                              const std::uint16_t& x,
                                              const std::uint16_t& y);
  void TextBGLine(const uint32_t& BG_ID);

  std::uint16_t getBgColorFromPalette(const std::uint32_t& paletteNumber,
                                      const std::uint32_t& colorID);
  std::uint16_t getBgColorFromPalette(const std::uint32_t& colorID);
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
