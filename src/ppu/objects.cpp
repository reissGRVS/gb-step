#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "ppu/pixel.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

const U16 OBJ_DIMENSIONS[4][3][2] = { { { 1, 1 }, { 2, 1 }, { 1, 2 } },
	{ { 2, 2 }, { 4, 1 }, { 1, 4 } },
	{ { 4, 4 }, { 4, 2 }, { 2, 4 } },
	{ { 8, 8 }, { 8, 4 }, { 4, 8 } } };
const U32 OAM_ENTRIES = 128;
const U16 MAX_SPRITE_X = 512, MAX_SPRITE_Y = 256;

void PPU::DrawObjects()
{
	for (S32 i = OAM_ENTRIES - 1; i >= 0; i--) {
		auto objAddress = OAM_START + (i * 8);
		auto objAttr0 = memory->GetHalf(objAddress);
		auto drawObjectEnabled = BIT_RANGE(objAttr0, 8, 9) != 0b10;
		if (drawObjectEnabled) {
			auto objAttr1 = memory->GetHalf(objAddress + 2);
			auto objAttr2 = memory->GetHalf(objAddress + 4);
			auto objAttrs = ObjAttributes(objAttr0, objAttr1, objAttr2);
			DrawObject(objAttrs);
		}
	}
}

void PPU::DrawObject(ObjAttributes objAttrs)
{
	auto [spriteWidth, spriteHeight] = OBJ_DIMENSIONS[objAttrs.attr1.b.objSize][objAttrs.attr0.b.objShape];
	auto topLeftTile = objAttrs.attr2.b.characterName;
	auto characterMapping = BIT_RANGE(GET_HALF(DISPCNT), 6, 6);
	auto halfTiles = objAttrs.attr0.b.colorsPalettes ? 2u : 1u;
	U16 colorDepth = objAttrs.attr0.b.colorsPalettes ? 8u : 4u;
	auto tileYIncrement = characterMapping ? (halfTiles * spriteWidth) : 0x20;

	for (U16 tileX = 0; tileX < spriteWidth; tileX++) {
		for (U16 tileY = 0; tileY < spriteHeight; tileY++) {
			U16 tileNumber = topLeftTile + (tileX * halfTiles) + (tileY * tileYIncrement);

			if (colorDepth == 8)
				tileNumber /= 2;

			auto actualTileX = tileX;
			if (objAttrs.attr1.b.horizontalFlip)
				actualTileX = spriteWidth - tileX - 1;

			auto actualTileY = tileY;
			if (objAttrs.attr1.b.verticalFlip)
				actualTileY = spriteHeight - tileY - 1;

			U16 x = (objAttrs.attr1.b.xCoord + TILE_PIXEL_WIDTH * actualTileX);
			U16 y = (objAttrs.attr0.b.yCoord + TILE_PIXEL_HEIGHT * actualTileY);
			auto tileInfo = TileInfo{ x,
				y,
				tileNumber,
				colorDepth,
				OBJ_START_ADDRESS,
				objAttrs.attr1.b.verticalFlip,
				objAttrs.attr1.b.horizontalFlip,
				objAttrs.attr2.b.paletteNumber,
				objAttrs.attr2.b.priority,
				(objAttrs.attr0.b.objMode == 1)};
			DrawTile(tileInfo);
		}
	}
}

void PPU::DrawTile(const TileInfo& info)
{
	const auto& [startX, startY, tileNumber, colorDepth, tileDataBase, verticalFlip, horizontalFlip, paletteNumber, priority, transparency] = info;
	U16 bldAlpha = GET_HALF(BLDALPHA);
	U8 eva = BIT_RANGE(bldAlpha, 0, 4);
	if (eva > 16) {eva = 16;}
	U8 evb = BIT_RANGE(bldAlpha, 8, 12);
	if (evb > 16) {evb = 16;}

	for (auto y = 0u; y < TILE_PIXEL_HEIGHT; y++) {
		for (auto x = 0u; x < TILE_PIXEL_WIDTH; x++) {
			auto totalX = (x + startX) % MAX_SPRITE_X;
			auto totalY = (y + startY) % MAX_SPRITE_Y;

			if (totalX < Screen::SCREEN_WIDTH && totalY < Screen::SCREEN_HEIGHT) {
				auto fbPos = totalX + (Screen::SCREEN_WIDTH * totalY);
				if (depth[fbPos] < priority) {
					continue;
				}

				auto pixel = GetTilePixel(tileNumber, x, y, colorDepth, tileDataBase,
					verticalFlip, horizontalFlip, paletteNumber, true);
				if (pixel) {
					auto pixelVal = pixel.value();
					if (transparency)
					{
						Pixel first{pixelVal};
						Pixel second{fb[fbPos]};
						first.Blend(second, eva, evb);
						pixelVal = first.Value();
					}
					fb[fbPos] = pixelVal;
					depth[fbPos] = priority;
				}
			}
		}
	}
}
