#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <iostream>


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
	auto absoluteY = (y + bgYOffset) % TEXT_BGMAP_SIZES[bgCnt.screenSize][1];
	auto framebufferY = Screen::SCREEN_WIDTH * y;
	
	auto mapY = absoluteY / TILE_PIXEL_HEIGHT;
	auto mapIndexY = ((mapY % TILE_AREA_HEIGHT) * TILE_AREA_WIDTH);
	
	auto x = 0u;
	while (x < Screen::SCREEN_WIDTH) {
		//TilePixelAtAbsoluteBGPosition
		// Get tile coords

		auto absoluteX = (x + bgXOffset) % TEXT_BGMAP_SIZES[bgCnt.screenSize][0];
		auto mapX = absoluteX / TILE_PIXEL_WIDTH;
		auto mapIndex = (mapX % TILE_AREA_WIDTH) + mapIndexY;
		auto pixelX = absoluteX % TILE_PIXEL_WIDTH;
		

		auto screenAreaAddressInc = GetScreenAreaOffset(mapX, mapY, bgCnt.screenSize);
		// Parse tile data
		auto bgMapEntry = memory->GetHalf(bgCnt.mapDataBase + screenAreaAddressInc + (mapIndex * BYTES_PER_ENTRY));
		auto tileNumber = BIT_RANGE(bgMapEntry, 0, 9);
		bool horizontalFlip = BIT_RANGE(bgMapEntry, 10, 10);
		bool verticalFlip = BIT_RANGE(bgMapEntry, 11, 11);
		auto paletteNumber = BIT_RANGE(bgMapEntry, 12, 15);

		//Draw tile line
		// Deal with flips 
		auto pixelY = absoluteY % TILE_PIXEL_HEIGHT;

		if (verticalFlip) {
			pixelY = TILE_PIXEL_HEIGHT - (pixelY + 1);
		}

		// Calculate position of tile pixel
		auto bytesPerTile = bgCnt.colorDepth * TILE_PIXEL_HEIGHT;
		auto startOfTileAddress = bgCnt.tileDataBase + (tileNumber * bytesPerTile);
		auto pixelsPerByte = 8 / bgCnt.colorDepth;


		for (auto px_ = pixelX; px_ < 8; px_++)
		{
			auto framebufferIndex = framebufferY + x;

			if (depth[framebufferIndex] <= bgCnt.priority) {
				x++;
				continue;
			}
			auto px = px_;
			if (horizontalFlip) {
				px = TILE_PIXEL_WIDTH - (px + 1);
			}

			if (px >= 8 || pixelY >= 8)
			{
				std::cerr << "fbindex oob" << std::endl;
				exit(01);
			}
			auto positionInTile = (px / pixelsPerByte) + (pixelY * bgCnt.colorDepth);
			auto pixelPalette = memory->GetByte(startOfTileAddress + positionInTile);

			std::optional<U16> pixel = {};
			if (bgCnt.colorDepth == 4) {
				if ((px % 2 == 0)) {
					pixelPalette = BIT_RANGE(pixelPalette, 0, 3);
				} else {
					pixelPalette = BIT_RANGE(pixelPalette, 4, 7);
				}

				if (pixelPalette != 0)
					pixel = GetBgColorFromSubPalette(paletteNumber, pixelPalette, false);
			} else // equal to 8
			{
				if (pixelPalette != 0)
					pixel = GetBgColorFromPalette(pixelPalette, false);
			}

			if (pixel) {
				fb[framebufferIndex] = pixel.value();
				depth[framebufferIndex] = bgCnt.priority;
			}

			x++;
			if (x >= Screen::SCREEN_WIDTH)
				break;
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
	return TILE_AREA_ADDRESS_INC * screenAreaId;
}
