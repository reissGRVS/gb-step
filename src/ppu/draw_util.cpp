#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

std::uint16_t PPU::GetBgColorFromSubPalette(const std::uint32_t& paletteNumber,
	const std::uint32_t& colorID,
	bool obj)
{
	return GetBgColorFromPalette(paletteNumber * 16u + colorID, obj);
}

std::uint16_t PPU::GetBgColorFromPalette(const std::uint32_t& colorID,
	bool obj)
{
	auto paletteStart = PRAM_START;
	if (obj)
		paletteStart += 0x200;
	return memory->GetHalf(colorID * 2 + paletteStart);
}

std::optional<std::uint16_t> PPU::GetTilePixel(std::uint16_t tileNumber,
	std::uint16_t x,
	std::uint16_t y,
	std::uint16_t colorDepth,
	std::uint32_t tileDataBase,
	bool verticalFlip,
	bool horizontalFlip,
	std::uint16_t paletteNumber,
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
