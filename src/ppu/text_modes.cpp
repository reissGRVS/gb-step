#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

const U32 BGHOFS[4] = { BG0HOFS, BG1HOFS, BG2HOFS, BG3HOFS };
const U32 BGVOFS[4] = { BG0VOFS, BG1VOFS, BG2VOFS, BG3VOFS };
const U16 TEXT_BGMAP_SIZES[4][2] = { { 256, 256 },
	{ 512, 256 },
	{ 256, 512 },
	{ 512, 512 } };

void PPU::TextBGLine(const U32& BG_ID)
{
	auto bgCnt = BGControlInfo(BG_ID, GET_HALF(BGCNT[BG_ID]));
	auto bgXOffset = GET_HALF(BGHOFS[BG_ID]) & NBIT_MASK(9);
	auto bgYOffset = GET_HALF(BGVOFS[BG_ID]) & NBIT_MASK(9);

	auto y = GET_HALF(VCOUNT);
	for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
		auto absoluteX = (x + bgXOffset) % TEXT_BGMAP_SIZES[bgCnt.screenSize][0];
		auto absoluteY = (y + bgYOffset) % TEXT_BGMAP_SIZES[bgCnt.screenSize][1];
		auto framebufferIndex = Screen::SCREEN_WIDTH * y + x;

		if (depth[framebufferIndex] <= bgCnt.priority) {
			continue;
		}

		auto pixel = TilePixelAtAbsoluteBGPosition(bgCnt, absoluteX, absoluteY);
		if (pixel) {
			fb[framebufferIndex] = pixel.value();
			depth[framebufferIndex] = bgCnt.priority;
		}
	}
}

std::optional<U16> PPU::TilePixelAtAbsoluteBGPosition(
	const BGControlInfo& bgCnt,
	const U16& x,
	const U16& y)
{
	// Get tile coords
	auto mapX = x / TILE_PIXEL_WIDTH;
	auto mapY = y / TILE_PIXEL_HEIGHT;
	auto mapIndex = (mapX % TILE_AREA_WIDTH) + ((mapY % TILE_AREA_HEIGHT) * TILE_AREA_WIDTH);
	auto pixelX = x % TILE_PIXEL_WIDTH;
	auto pixelY = y % TILE_PIXEL_HEIGHT;

	auto screenAreaAddressInc = GetScreenAreaOffset(mapX, mapY, bgCnt.screenSize);
	// Parse tile data
	auto bgMapEntry = memory->GetHalf(bgCnt.mapDataBase + screenAreaAddressInc + (mapIndex * BYTES_PER_ENTRY));
	auto tileNumber = BIT_RANGE(bgMapEntry, 0, 9);
	bool horizontalFlip = BIT_RANGE(bgMapEntry, 10, 10);
	bool verticalFlip = BIT_RANGE(bgMapEntry, 11, 11);
	auto paletteNumber = BIT_RANGE(bgMapEntry, 12, 15);

	return GetTilePixel(tileNumber, pixelX, pixelY, bgCnt.colorDepth,
		bgCnt.tileDataBase, verticalFlip, horizontalFlip,
		paletteNumber, false);
}

U32 PPU::GetScreenAreaOffset(U32 mapX,
	U32 mapY,
	U8 screenSize)
{
	auto screenX = mapX / TILE_AREA_WIDTH;
	auto screenY = mapY / TILE_AREA_HEIGHT;
	auto screenAreaId = 0;
	switch (screenSize) {
	case 0:
		break;
	case 1:
		screenAreaId = screenX;
		break;
	case 2:
		screenAreaId = screenY;
		break;
	case 3:
		screenAreaId = screenX + screenY * 2;
		break;
	}
	return TILE_AREA_ADDRESS_INC * screenAreaId;
}