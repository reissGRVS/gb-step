#pragma once

#include "memory/memory.hpp"
#include "ppu/bg_control_info.hpp"
#include "ppu/obj_attributes.hpp"
#include "ppu/tile_info.hpp"
#include "screen.hpp"

#include <optional>
#include <vector>
class PPU {
  enum State { Visible, HBlank, VBlank };

 public:
  PPU(std::shared_ptr<Memory> memory_, Screen& screen_)
      : memory(memory_), screen(screen_) {}
  void Execute(std::uint32_t ticks);

  std::function<void(bool)> HBlankCallback = [](bool) { return; };
  std::function<void(bool)> VBlankCallback = [](bool) { return; };

 private:
  // State Management
  void ToHBlank();
  void OnHBlankFinish();
  void ToVBlank();
  void OnVBlankLineFinish();
  std::uint16_t GetDispStat(std::uint8_t bit);
  void UpdateDispStat(std::uint8_t bit, bool set);
  std::uint16_t IncrementVCount();

  // Draw Control
  void DrawLine();
  uint8_t GetLayerPriority(uint8_t layer);
  std::vector<uint8_t> GetBGDrawOrder(std::vector<uint8_t> layers,
                                      uint8_t screenDisplay);

  // Objects
  void DrawObjects();
  void DrawObject(ObjAttributes objAttrs);
  void DrawTile(const TileInfo& info);

  // Text Mode
  void TextBGLine(const uint32_t& BG_ID);
  std::optional<std::uint16_t> TilePixelAtAbsoluteBGPosition(
      const BGControlInfo& bgCnt,
      const std::uint16_t& x,
      const std::uint16_t& y);
  std::uint32_t GetScreenAreaOffset(std::uint32_t mapX,
                                    std::uint32_t mapY,
                                    std::uint_fast8_t screenSize);

  // Draw Utils
  std::optional<std::uint16_t> GetTilePixel(std::uint16_t tileNumber,
                                            std::uint16_t x,
                                            std::uint16_t y,
                                            std::uint16_t colorDepth,
                                            std::uint32_t tileDataBase,
                                            bool verticalFlip,
                                            bool horizontalFlip,
                                            std::uint16_t paletteNumber,
                                            bool obj);
  std::uint16_t GetBgColorFromSubPalette(const std::uint32_t& paletteNumber,
                                         const std::uint32_t& colorID,
                                         bool obj = false);
  std::uint16_t GetBgColorFromPalette(const std::uint32_t& colorID,
                                      bool obj = false);

  std::shared_ptr<Memory> memory;
  Screen& screen;

  Screen::Framebuffer depth{4};
  Screen::Framebuffer fb{};
  State state = Visible;
  std::uint32_t tickCount = 0;

  const std::uint16_t TILE_PIXEL_HEIGHT = 8, TILE_PIXEL_WIDTH = 8,
                      TILE_AREA_HEIGHT = 32, TILE_AREA_WIDTH = 32;

  const std::uint32_t TILE_AREA_ADDRESS_INC = 0x800, BYTES_PER_ENTRY = 2,
                      OBJ_START_ADDRESS = 0x06010000;

  const std::uint32_t BGCNT[4] = {BG0CNT, BG1CNT, BG2CNT, BG3CNT};

  const std::uint16_t MAX_DEPTH = 4;
};
