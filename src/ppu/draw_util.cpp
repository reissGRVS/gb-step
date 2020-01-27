#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

U16 PPU::GetBgColorFromSubPalette(const U32& paletteNumber,
	const U32& colorID,
	bool obj)
{
	return GetBgColorFromPalette(paletteNumber * 16u + colorID, obj);
}

U16 PPU::GetBgColorFromPalette(const U32& colorID,
	bool obj)
{
	auto paletteStart = PRAM_START;
	if (obj)
		paletteStart += 0x200;
	return memory->GetHalf(colorID * 2 + paletteStart);
}
