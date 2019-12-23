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

std::optional<U16> PPU::GetTilePixel(U16 tileNumber,
	U16 x,
	U16 y,
	U16 colorDepth,
	U32 tileDataBase,
	bool verticalFlip,
	bool horizontalFlip,
	U16 paletteNumber,
	bool obj)
{
	// Deal with flips
	if (verticalFlip) {
		y = TILE_PIXEL_HEIGHT - (y + 1);
	}
	if (horizontalFlip) {
		x = TILE_PIXEL_WIDTH - (x + 1);
	}

	// Calculate position of tile pixel
	auto bytesPerTile = colorDepth * TILE_PIXEL_HEIGHT;
	auto startOfTileAddress = tileDataBase + (tileNumber * bytesPerTile);
	auto pixelsPerByte = 8 / colorDepth;
	auto positionInTile = (x / pixelsPerByte) + (y * colorDepth);
	auto pixelPalette = memory->GetByte(startOfTileAddress + positionInTile);
	if (colorDepth == 4) {
		if ((x % 2 == 0)) {
			pixelPalette = BIT_RANGE(pixelPalette, 0, 3);
		} else {
			pixelPalette = BIT_RANGE(pixelPalette, 4, 7);
		}

		if (pixelPalette == 0)
			return {};
		return GetBgColorFromSubPalette(paletteNumber, pixelPalette, obj);
	} else // equal to 8
	{
		if (pixelPalette == 0)
			return {};
		return GetBgColorFromPalette(pixelPalette, obj);
	}
}
