#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

const uint16_t OBJ_DIMENSIONS[4][3][2] = { { { 1, 1 }, { 2, 1 }, { 1, 2 } },
	{ { 2, 2 }, { 4, 1 }, { 1, 4 } },
	{ { 4, 4 }, { 4, 2 }, { 2, 4 } },
	{ { 8, 8 }, { 8, 4 }, { 4, 8 } } };
const std::uint32_t OAM_ENTRIES = 128;

void PPU::DrawObjects()
{
	for (std::int32_t i = OAM_ENTRIES - 1; i >= 0; i--) {
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
	auto characterMapping = BIT_RANGE(memory->GetHalf(DISPCNT), 6, 6);
	auto halfTiles = objAttrs.attr0.b.colorsPalettes ? 2u : 1u;
	std::uint16_t colorDepth = objAttrs.attr0.b.colorsPalettes ? 8u : 4u;
	auto tileYIncrement = characterMapping ? (halfTiles * spriteWidth) : 0x20;

	for (std::uint16_t tileX = 0; tileX < spriteWidth; tileX++) {
		for (std::uint16_t tileY = 0; tileY < spriteHeight; tileY++) {
			std::uint16_t tileNumber = topLeftTile + (tileX * halfTiles) + (tileY * tileYIncrement);

			if (colorDepth == 8)
				tileNumber /= 2;

			auto actualTileX = tileX;
			if (objAttrs.attr1.b.horizontalFlip)
				actualTileX = spriteWidth - tileX - 1;

			auto actualTileY = tileY;
			if (objAttrs.attr1.b.verticalFlip)
				actualTileY = spriteHeight - tileY - 1;

			std::uint16_t x = objAttrs.attr1.b.xCoord + TILE_PIXEL_WIDTH * actualTileX;
			std::uint16_t y = objAttrs.attr0.b.yCoord + TILE_PIXEL_HEIGHT * actualTileY;
			auto tileInfo = TileInfo{ x,
				y,
				tileNumber,
				colorDepth,
				OBJ_START_ADDRESS,
				objAttrs.attr1.b.verticalFlip,
				objAttrs.attr1.b.horizontalFlip,
				objAttrs.attr2.b.paletteNumber,
				objAttrs.attr2.b.priority };
			DrawTile(tileInfo);
		}
	}
}

void PPU::DrawTile(const TileInfo& info)
{
	const auto& [startX, startY, tileNumber, colorDepth, tileDataBase, verticalFlip, horizontalFlip, paletteNumber, priority] = info;

	for (auto x = 0u; x < TILE_PIXEL_WIDTH; x++) {
		for (auto y = 0u; y < TILE_PIXEL_HEIGHT; y++) {
			auto totalX = (x + startX);
			auto totalY = (y + startY);

			if (totalX < Screen::SCREEN_WIDTH && totalY < Screen::SCREEN_HEIGHT) {
				auto fbPos = totalX + (Screen::SCREEN_WIDTH * totalY);
				if (depth[fbPos] < priority) {
					continue;
				}

				auto pixel = GetTilePixel(tileNumber, x, y, colorDepth, tileDataBase,
					verticalFlip, horizontalFlip, paletteNumber, true);
				if (pixel) {
					fb[fbPos] = pixel.value();
					depth[fbPos] = priority;
				}
			}
		}
	}
}
