#pragma once
#include <cstdint>

#define REGION_INFO(NAME, SIZE, START, MASK) \
  static const std::uint32_t NAME##_size = SIZE; \
  static const std::uint32_t NAME##_start = START; \
  static const std::uint32_t NAME##_mask = MASK;

static const std::uint32_t page_mask = 0xFFFFFF;
REGION_INFO(bios, 0x04000, 0x0000000, 0x03FFF)
REGION_INFO(wramb, 0x40000, 0x2000000, 0x3FFFF)
REGION_INFO(wramc, 0x08000, 0x3000000, 0x07FFF)
REGION_INFO(ioreg, 0x00400, 0x4000000, 0x003FF)
REGION_INFO(pram, 0x00400, 0x5000000, 0x003FF)
REGION_INFO(vram, 0x18000, 0x6000000, 0x1FFFF)
REGION_INFO(oam, 0x00400, 0x7000000, 0x003FF)
REGION_INFO(rom, 0x2000000, 0x8000000, 0x1FFFFFF)

const std::uint32_t DISPCNT = 0x4000000;
const std::uint32_t DISPSTAT = 0x4000004;
const std::uint32_t VCOUNT = 0x4000006;

const std::uint32_t KEYINPUT = 0x4000130;

const std::uint32_t IE = 0x4000200;
const std::uint32_t IF = 0x4000202;
const std::uint32_t IME = 0x4000208;
