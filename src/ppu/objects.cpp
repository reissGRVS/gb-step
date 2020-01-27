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
	auto startX = objAttrs.attr1.b.xCoord;
	auto startY = objAttrs.attr0.b.yCoord;
	auto paletteNumber = objAttrs.attr2.b.paletteNumber;
	

	const auto TILE_PIXEL_TOTAL = TILE_PIXEL_HEIGHT * TILE_PIXEL_WIDTH;
	const auto ROW_TILE_PIXEL_TOTAL = TILE_PIXEL_TOTAL * spriteWidth;
	const auto SPRITE_PIXEL_WIDTH = TILE_PIXEL_WIDTH * spriteWidth;
	const auto SPRITE_PIXEL_HEIGHT = TILE_PIXEL_HEIGHT * spriteHeight;
	//Transfer sprite data to tempSprite
	auto tempSpriteIndexStart = 0u;
	for (U16 tileY = 0; tileY < spriteHeight; tileY++) {
		tempSpriteIndexStart = tileY * ROW_TILE_PIXEL_TOTAL;
		for (U16 tileX = 0; tileX < spriteWidth; tileX++) {
			auto tempSpriteIndex = tempSpriteIndexStart;

			// Find tile data
			U16 tileNumber = topLeftTile + (tileX * halfTiles) + (tileY * tileYIncrement);
			if (colorDepth == 8)
				tileNumber /= 2;
			auto startOfTileAddress = OBJ_START_ADDRESS + (tileNumber * colorDepth * TILE_PIXEL_HEIGHT);
			
			// Transfer tile data to tempSprite
			auto positionInTile = 0u;
			for (auto pixy = 0u; pixy < TILE_PIXEL_HEIGHT; pixy++) {
				for (auto pixx = 0u; pixx < TILE_PIXEL_WIDTH; pixx++) {
					const auto pixelPalette = memory->GetByte(startOfTileAddress + positionInTile);

					if (colorDepth == 4) {
						auto color1 = BIT_RANGE(pixelPalette, 0, 3);
						if (color1 == 0)
							tempSprite[tempSpriteIndex] = {};
						else
							tempSprite[tempSpriteIndex] = GetBgColorFromSubPalette(paletteNumber, color1, true);
						
						tempSpriteIndex++;
						pixx++;
						
						auto color2 = BIT_RANGE(pixelPalette, 4, 7);
						if (color2 == 0)
							tempSprite[tempSpriteIndex] = {};
						else
							tempSprite[tempSpriteIndex] = GetBgColorFromSubPalette(paletteNumber, color2, true);
						tempSpriteIndex++;

					} else // equal to 8
					{
						if (pixelPalette == 0)
							tempSprite[tempSpriteIndex] = {};
						else
							tempSprite[tempSpriteIndex] = GetBgColorFromPalette(pixelPalette, true);
						tempSpriteIndex++;
					}

					positionInTile++;
				}
				tempSpriteIndex += SPRITE_PIXEL_WIDTH - TILE_PIXEL_WIDTH;
			}
			tempSpriteIndexStart += TILE_PIXEL_WIDTH;
		}
	}

	//Set up affine transformations
	auto dx = 256;
	auto dmx = 0;
	auto dy = 0;
	auto dmy = 256;

	if (objAttrs.attr0.b.rotationScalingFlag)
	{
		U32 ParamLocationBase = OAM_START + (0x20 * objAttrs.GetRotScaleParams());
		dx = 	(S16)memory->GetHalf(ParamLocationBase+0x06u);
		dmx = 	(S16)memory->GetHalf(ParamLocationBase+0x0Eu);
		dy = 	(S16)memory->GetHalf(ParamLocationBase+0x16u);
		dmy = 	(S16)memory->GetHalf(ParamLocationBase+0x1Eu);
	}
	else
	{
		if (objAttrs.attr1.b.horizontalFlip)
		{
			dx = -256;
		}
		if (objAttrs.attr1.b.verticalFlip)
		{
			dmy = -256;
		}
	}
	

	const auto DBL_SPRITE_HEIGHT = SPRITE_PIXEL_HEIGHT * (objAttrs.attr0.b.objDisable ? 2 : 1);
	const auto DBL_SPRITE_WIDTH = SPRITE_PIXEL_WIDTH * (objAttrs.attr0.b.objDisable ? 2 : 1);
	const auto HALF_SPRITE_HEIGHT = DBL_SPRITE_HEIGHT / 2;
	const auto HALF_SPRITE_WIDTH = DBL_SPRITE_WIDTH / 2;
	auto rotY = SPRITE_PIXEL_HEIGHT / 2;
	auto rotX = SPRITE_PIXEL_WIDTH / 2;

	//Transfer tempSprite to framebuffer
	for (U16 y = 0; y < DBL_SPRITE_HEIGHT; y++) {
		U16 fbY = (startY + y) % MAX_SPRITE_Y;
		for (U16 x = 0; x < DBL_SPRITE_WIDTH; x++) {
			S16 xAdj = x - HALF_SPRITE_WIDTH;
			S16 yAdj = y - HALF_SPRITE_HEIGHT;
			S16 newX = (dx*xAdj + dmx*yAdj)/256 + rotX;
			S16 newY = (dy*xAdj + dmy*yAdj)/256 + rotY;

			if (newX >= SPRITE_PIXEL_WIDTH || newY >= SPRITE_PIXEL_HEIGHT || newX < 0 || newY < 0) continue;
			auto tempSpriteIndex = newY * SPRITE_PIXEL_WIDTH + newX;

			U16 fbX = (startX + x) % MAX_SPRITE_X;
			if (fbX < Screen::SCREEN_WIDTH && fbY < Screen::SCREEN_HEIGHT) {
				auto fbPos = fbX + (Screen::SCREEN_WIDTH * fbY);
				if (depth[fbPos] < objAttrs.attr2.b.priority) {
					continue;
				}

				auto pixel = tempSprite[tempSpriteIndex];

				if (pixel.has_value())
				{
					auto pixelVal = pixel.value();
					if (objAttrs.attr0.b.objMode == 1)
					{
						Pixel first{pixelVal};
						if (secondtarget[fbPos])
						{
							Pixel second{fb[fbPos]};
							first.Blend(second, eva, evb);
						}
						pixelVal = first.Value();
					}
					fb[fbPos] = pixelVal;
				}
			}
		}
	}
}