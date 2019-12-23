#pragma once

#include <cstdint>

struct TileInfo {
  const std::uint16_t startX;
  const std::uint16_t startY;
  const std::uint16_t tileNumber;
  const std::uint16_t colorDepth;
  const std::uint32_t tileDataBase;
  const std::uint8_t verticalFlip;
  const std::uint8_t horizontalFlip;
  const std::uint8_t paletteNumber;
  const std::uint8_t priority;
};