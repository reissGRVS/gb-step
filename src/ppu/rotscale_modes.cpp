#include "memory/regions.hpp"
#include "ppu/ppu.hpp"

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
	bgCnt[BG_ID].UpdateValue(GET_HALF(BGCNT[BG_ID]));

	auto dx = (S16)GET_HALF(BGPA[BG_ID - 2]);
	auto dmx = (S16)GET_HALF(BGPB[BG_ID - 2]);
	auto dy = (S16)GET_HALF(BGPC[BG_ID - 2]);
	auto dmy = (S16)GET_HALF(BGPD[BG_ID - 2]);

	auto [maxX, maxY] = ROTSCALE_BGMAP_SIZES[bgCnt[BG_ID].screenSize];

	auto y = GET_HALF(VCOUNT);
	//floor to mosaic
	if (bgCnt[BG_ID].mosaic)
		y = (y / mosaic.bgVSize) * mosaic.bgVSize;

	auto refx = bgXRef[BG_ID - 2] + (y * (S32)dmx);
	auto refy = bgYRef[BG_ID - 2] + (y * (S32)dmy);
	for (auto rowX = 0u; rowX < Screen::SCREEN_WIDTH; rowX++) {

		//Calculate Position in map
		auto x = refx / 256;
		auto y = refy / 256;
		bool negativeCoords = (refx < 0) || (refy < 0);
		refx += dx;
		refy += dy;

		if (x >= maxX || y >= maxY || negativeCoords) {
			if (bgCnt[BG_ID].wrapAround) {
				x %= maxX;
				y %= maxY;
				if (x < 0)
					x += maxX;
				if (y < 0)
					y += maxY;
			} else {
				continue;
			}
		}
		auto mapX = x / TILE_PIXEL_WIDTH;
		auto tileX = x % TILE_PIXEL_WIDTH;
		auto mapY = y / TILE_PIXEL_HEIGHT;
		auto tileY = y % TILE_PIXEL_HEIGHT;
		auto mapWidth = maxX / TILE_PIXEL_WIDTH;
		auto mapIndex = (mapY * mapWidth) + mapX;

		//Draw Pixel
		auto tileNumber = memory->GetByte(bgCnt[BG_ID].mapDataBase + mapIndex);
		const U16 BYTES_PER_TILE = TILE_PIXEL_WIDTH * TILE_PIXEL_HEIGHT;
		auto pixelAddress = bgCnt[BG_ID].tileDataBase + (tileNumber * BYTES_PER_TILE) + (tileY * TILE_PIXEL_WIDTH) + tileX;
		FetchDecode8BitPixel(pixelAddress, rows[BG_ID][rowX], false);
	}
}
