#include "memory/regions.hpp"
#include "platform/logging.hpp"
#include "ppu/ppu.hpp"
#include "utils.hpp"

const U32 BGHOFS[4] = { BG0HOFS, BG1HOFS, BG2HOFS, BG3HOFS };
const U32 BGVOFS[4] = { BG0VOFS, BG1VOFS, BG2VOFS, BG3VOFS };
const U16 TEXT_BGMAP_SIZES[4][2] = { { 256, 256 },
	{ 512, 256 },
	{ 256, 512 },
	{ 512, 512 } };
const U32 BYTES_PER_ENTRY = 2;

void PPU::TextBGLine(const U32& BG_ID)
{
	bgCnt[BG_ID] = BGControlInfo(BG_ID, GET_HALF(BGCNT[BG_ID]));

	auto bgXOffset = GET_HALF(BGHOFS[BG_ID]) & NBIT_MASK(9);
	auto bgYOffset = GET_HALF(BGVOFS[BG_ID]) & NBIT_MASK(9);

	auto y = GET_HALF(VCOUNT);

	//floor to mosaic
	if (bgCnt[BG_ID].mosaic)
		y = (y / mosaic.bgVSize) * mosaic.bgVSize;

	auto absoluteY = (y + bgYOffset) % TEXT_BGMAP_SIZES[bgCnt[BG_ID].screenSize][1];

	auto mapY = absoluteY / TILE_PIXEL_HEIGHT;
	auto mapIndexY = ((mapY % TILE_AREA_HEIGHT) * TILE_AREA_WIDTH);
	auto pixelY = absoluteY % TILE_PIXEL_HEIGHT;
	auto flippedPixelY = TILE_PIXEL_HEIGHT - (pixelY + 1);
	auto bytesPerTile = bgCnt[BG_ID].colorDepth * TILE_PIXEL_HEIGHT;
	auto pixelsPerByte = 8 / bgCnt[BG_ID].colorDepth;

	auto x = 0u;
	while (x < Screen::SCREEN_WIDTH) {
		//TilePixelAtAbsoluteBGPosition
		// Get tile coords

		auto absoluteX = (x + bgXOffset) % TEXT_BGMAP_SIZES[bgCnt[BG_ID].screenSize][0];
		auto mapX = absoluteX / TILE_PIXEL_WIDTH;
		auto mapIndex = (mapX % TILE_AREA_WIDTH) + mapIndexY;
		auto pixelX = absoluteX % TILE_PIXEL_WIDTH;

		auto screenAreaAddressInc = GetScreenAreaOffset(mapX, mapY, bgCnt[BG_ID].screenSize);
		// Parse tile data
		auto bgMapEntry = memory->GetHalf(bgCnt[BG_ID].mapDataBase + screenAreaAddressInc + (mapIndex * BYTES_PER_ENTRY));
		auto tileNumber = BIT_RANGE(bgMapEntry, 0, 9);
		bool horizontalFlip = BIT_RANGE(bgMapEntry, 10, 10);
		bool verticalFlip = BIT_RANGE(bgMapEntry, 11, 11);
		auto paletteNumber = BIT_RANGE(bgMapEntry, 12, 15);

		//Draw tile line
		auto py = verticalFlip ? flippedPixelY : pixelY;

		// Calculate position of tile pixel
		auto startOfTileAddress = bgCnt[BG_ID].tileDataBase + (tileNumber * bytesPerTile);
		auto positionInTileY = (py * bgCnt[BG_ID].colorDepth);

		for (auto px_ = pixelX; px_ < 8; px_++) {
			if (x >= Screen::SCREEN_WIDTH)
				break;
			auto px = px_;
			if (horizontalFlip) {
				px = TILE_PIXEL_WIDTH - (px + 1);
			}

			auto pixelAddress = startOfTileAddress + (px / pixelsPerByte) + positionInTileY;
			if (bgCnt[BG_ID].colorDepth == 4) {
				FetchDecode4BitPixel(pixelAddress, rows[BG_ID][x], paletteNumber, (px % 2 == 0), false);
			} else // equal to 8
			{
				FetchDecode8BitPixel(pixelAddress, rows[BG_ID][x], false);
			}
			x++;
		}
	}
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

	if (screenAreaId > 3) {
		LOG_ERROR("screenAreaID oob")
		exit(01);
	}

	return TILE_AREA_ADDRESS_INC * screenAreaId;
}
