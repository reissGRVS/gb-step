#pragma once

#include "memory.hpp"
#include "ppu/bg_control_info.hpp"
#include "ppu/obj_attributes.hpp"
#include "screen.hpp"

#include <optional>
#include <vector>

class PPU {
  enum State { Visible, HBlank, VBlank };

 public:
  PPU(std::shared_ptr<Memory> memory_, Screen& screen_)
      : memory(memory_), screen(screen_) {}
  void Execute(std::uint32_t ticks);

  std::function<void()> HBlankCallback = []() { return; };

  std::function<void()> VBlankCallback = []() { return; };

 private:
  void DrawLine();
  void DrawObjects();
  void DrawObject(ObjAttributes objAttrs);
  void DrawTile(std::uint16_t startX,
                std::uint16_t startY,
                std::uint16_t tileNumber,
                std::uint16_t colorDepth,
                std::uint32_t tileDataBase,
                bool verticalFlip,
                bool horizontalFlip,
                std::uint16_t paletteNumber,
                std::uint16_t priority);

  std::optional<std::uint16_t> GetTilePixel(std::uint16_t tileNumber,
                                            std::uint16_t x,
                                            std::uint16_t y,
                                            std::uint16_t colorDepth,
                                            std::uint32_t tileDataBase,
                                            bool verticalFlip,
                                            bool horizontalFlip,
                                            std::uint16_t paletteNumber,
                                            bool obj);

  std::optional<std::uint16_t> TilePixelAtAbsoluteBGPosition(
      const BGControlInfo& bgCnt,
      const std::uint16_t& x,
      const std::uint16_t& y);
  void TextBGLine(const uint32_t& BG_ID);

  uint8_t GetLayerPriority(uint8_t layer);
  std::vector<uint8_t> GetBGDrawOrder(std::vector<uint8_t> layers,
                                      uint8_t screenDisplay);

  std::uint16_t GetBgColorFromSubPalette(const std::uint32_t& paletteNumber,
                                         const std::uint32_t& colorID,
                                         bool obj = false);
  std::uint16_t GetBgColorFromPalette(const std::uint32_t& colorID,
                                      bool obj = false);
  std::uint8_t GetByte(const std::uint32_t& address);
  std::uint16_t GetHalf(const std::uint32_t& address);
  void SetHalf(const std::uint32_t& address, const std::uint16_t& value);

  std::shared_ptr<Memory> memory;
  Screen& screen;
  //   std::string class_id = "dwoadjoawjc";
  Screen::Framebuffer depth{4};
  Screen::Framebuffer fb{};
  State state = Visible;
  std::uint32_t tickCount;

  const std::uint32_t CYCLES_PER_VISIBLE = 960, CYCLES_PER_HBLANK = 272,
                      CYCLES_PER_LINE = CYCLES_PER_VISIBLE + CYCLES_PER_HBLANK,
                      VISIBLE_LINES = 160, VBLANK_LINES = 68,
                      TOTAL_LINES = VISIBLE_LINES + VBLANK_LINES,
                      CYCLES_PER_VBLANK = CYCLES_PER_LINE * VBLANK_LINES;

  const std::uint16_t MAX_DEPTH = 4;
};