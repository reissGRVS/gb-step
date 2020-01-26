#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

const U16 ROTSCALE_BGMAP_SIZES[4][2] = { { 128, 128 },
	{ 256, 256 },
	{ 512, 512 },
	{ 1024, 1024 } };

const U32 BGPA[2] = { BG2PA, BG3PA };
const U32 BGPB[2] = { BG2PB, BG3PB };
const U32 BGPC[2] = { BG2PC, BG3PC };
const U32 BGPD[2] = { BG2PD, BG3PD };

void PPU::RotScaleBGLine(const U32& BG_ID)
{
	auto bgCnt = BGControlInfo(BG_ID,  memory->GetHalf(BGCNT[BG_ID]));

	auto dx = (S16)memory->GetHalf(BGPA[BG_ID-2]);
	auto dmx = (S16)memory->GetHalf(BGPB[BG_ID-2]);
	auto dy = (S16)memory->GetHalf(BGPC[BG_ID-2]);
	auto dmy = (S16)memory->GetHalf(BGPD[BG_ID-2]);

	auto [maxX, maxY] = ROTSCALE_BGMAP_SIZES[bgCnt.screenSize];

	auto refx = bgXRef[BG_ID-2];
	auto refy = bgYRef[BG_ID-2];

	for (auto rowX = 0u; rowX < Screen::SCREEN_WIDTH; rowX++) {
		auto x = refx >> 8;
		auto y = refy >> 8;
		refx += dx;
		refy += dy;

		if (x > maxX || y > maxY || x < 0 || y < 0)
		{
			if (bgCnt.wrapAround)
			{
				x %= maxX;
				y %= maxY;
				if (x < 0) x+=maxX;
				if (y < 0) y+=maxY;
			}
			else
			{
				continue;
			}
		}

		auto mapX = x / TILE_PIXEL_WIDTH;
		auto tileX = x % TILE_PIXEL_WIDTH;
		
		auto mapY = y / TILE_PIXEL_HEIGHT;
		auto tileY = y % TILE_PIXEL_HEIGHT;

		auto mapWidth = maxX / TILE_PIXEL_WIDTH;
		auto mapIndex = (mapY * mapWidth) + mapX; 
		auto tileNumber = memory->GetByte(bgCnt.mapDataBase + mapIndex);

		const U16 BYTES_PER_TILE = TILE_PIXEL_WIDTH * TILE_PIXEL_HEIGHT;
		auto pixelAddress = bgCnt.tileDataBase + (tileNumber * BYTES_PER_TILE) + (tileY * TILE_PIXEL_WIDTH) + tileX;
		auto pixelPalette = memory->GetByte(pixelAddress);
		if (pixelPalette != 0)
		{
			rows[BG_ID][rowX] = GetBgColorFromPalette(pixelPalette, false);
		}
	}

	bgXRef[BG_ID-2]+=dmx;
	bgYRef[BG_ID-2]+=dmy;
}
