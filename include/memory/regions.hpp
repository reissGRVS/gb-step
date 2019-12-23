#pragma once
#include "int.hpp"
#include <unordered_map>

#define REGION_INFO(NAME, SIZE, START, MASK) \
	static const U32 NAME##_SIZE = SIZE;     \
	static const U32 NAME##_START = START;   \
	static const U32 NAME##_MASK = MASK;

#define IOREG_INFO(NAME, ADDRESS)    \
	static const U32 NAME = ADDRESS; \
	static const std::string NAME##_STR = #NAME;

static const U32 PAGE_MASK = 0xFFFFFF;
REGION_INFO(BIOS, 0x04000, 0x0000000, 0x03FFF)
REGION_INFO(WRAMB, 0x40000, 0x2000000, 0x3FFFF)
REGION_INFO(WRAMC, 0x08000, 0x3000000, 0x07FFF)
REGION_INFO(IOREG, 0x00400, 0x4000000, 0x003FF)
REGION_INFO(PRAM, 0x00400, 0x5000000, 0x003FF)
REGION_INFO(VRAM, 0x18000, 0x6000000, 0x1FFFF)
REGION_INFO(OAM, 0x00400, 0x7000000, 0x003FF)
REGION_INFO(ROM, 0x2000000, 0x8000000, 0x1FFFFFF)
REGION_INFO(SRAM, 0x10000, 0xE000000, 0x0FFFF)
REGION_INFO(FLASH_BANK, 0x10000, 0xE000000, 0x0FFFF)

IOREG_INFO(DISPCNT, 0x4000000)
IOREG_INFO(DISPSTAT, 0x4000004)
IOREG_INFO(VCOUNT, 0x4000006)
IOREG_INFO(BG0CNT, 0x4000008)
IOREG_INFO(BG1CNT, 0x400000A)
IOREG_INFO(BG2CNT, 0x400000C)
IOREG_INFO(BG3CNT, 0x400000E)
IOREG_INFO(BG0HOFS, 0x4000010)
IOREG_INFO(BG0VOFS, 0x4000012)
IOREG_INFO(BG1HOFS, 0x4000014)
IOREG_INFO(BG1VOFS, 0x4000016)
IOREG_INFO(BG2HOFS, 0x4000018)
IOREG_INFO(BG2VOFS, 0x400001A)
IOREG_INFO(BG3HOFS, 0x400001C)
IOREG_INFO(BG3VOFS, 0x400001E)

IOREG_INFO(KEYINPUT, 0x4000130)
IOREG_INFO(IE, 0x4000200)
IOREG_INFO(IF, 0x4000202)
IOREG_INFO(WAITCNT, 0x4000204)
IOREG_INFO(IME, 0x4000208)

IOREG_INFO(DMA0SAD, 0x40000B0)
IOREG_INFO(DMA0DAD, 0x40000B4)
IOREG_INFO(DMA0CNT_L, 0x40000B8)
IOREG_INFO(DMA0CNT_H, 0x40000BA)
IOREG_INFO(DMA1CNT_H, 0x40000C6)
IOREG_INFO(DMA2CNT_H, 0x40000D2)
IOREG_INFO(DMA3CNT_H, 0x40000DE)

IOREG_INFO(TM0CNT_L, 0x4000100)
IOREG_INFO(TM0CNT_H, 0x4000102)
IOREG_INFO(TM1CNT_L, 0x4000104)
IOREG_INFO(TM1CNT_H, 0x4000106)
IOREG_INFO(TM2CNT_L, 0x4000108)
IOREG_INFO(TM2CNT_H, 0x400010A)
IOREG_INFO(TM3CNT_L, 0x400010C)
IOREG_INFO(TM3CNT_H, 0x400010E)
