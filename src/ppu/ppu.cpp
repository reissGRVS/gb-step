#include "ppu/ppu.hpp"
#include <map>
#include "memory_regions.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

const std::uint32_t TILE_PIXEL_HEIGHT = 8, TILE_PIXEL_WIDTH = 8,
                    TILE_AREA_HEIGHT = 32, TILE_AREA_WIDTH = 32,
                    TILE_AREA_ADDRESS_INC = 0x800, BYTES_PER_ENTRY = 2,
                    OBJ_START_ADDRESS = 0x06010000;

const std::uint16_t TEXT_BGMAP_SIZES[4][2] = {{256, 256},
                                              {512, 256},
                                              {256, 512},
                                              {512, 512}};
const std::uint32_t BGCNT[4] = {BG0CNT, BG1CNT, BG2CNT, BG3CNT};
const std::uint32_t BGHOFS[4] = {BG0HOFS, BG1HOFS, BG2HOFS, BG3HOFS};
const std::uint32_t BGVOFS[4] = {BG0VOFS, BG1VOFS, BG2VOFS, BG3VOFS};

void PPU::Execute(std::uint32_t ticks) {
  tickCount += ticks;

  // https://problemkaputt.de/gbatek.htm#lcddimensionsandtimings
  switch (state) {
	case Visible: {
	  if (tickCount > CYCLES_PER_VISIBLE) {
		tickCount = tickCount - CYCLES_PER_VISIBLE;
		state = HBlank;

		DrawLine();
		// Set HBlank flag and Request Interrupt
		auto dispStat = GetHalf(DISPSTAT);
		BIT_SET(dispStat, 1);
		SetHalf(DISPSTAT, dispStat);
		if (BIT_RANGE(dispStat, 4, 4)) {
		  spdlog::get("std")->debug("HBlank IntReq");
		  auto intReq = GetHalf(IF);
		  BIT_SET(intReq, 1);
		  SetHalf(IF, intReq);
		}
	  }
	  break;
	}
	case HBlank: {
	  if (tickCount > CYCLES_PER_HBLANK) {
		tickCount = tickCount - CYCLES_PER_HBLANK;

		// Set HBlank flag to 0
		auto dispStat = GetHalf(DISPSTAT);
		BIT_CLEAR(dispStat, 1);
		SetHalf(DISPSTAT, dispStat);

		auto vCount = GetHalf(VCOUNT);
		vCount++;
		SetHalf(VCOUNT, vCount);
		spdlog::get("std")->debug("HBlank Line {}", vCount);

		if (vCount >= VISIBLE_LINES) {
		  state = VBlank;

		  DrawObjects();
		  screen.render(fb);
		  fb.fill(0);

		  // Set VBlank flag and Request Interrupt
		  BIT_SET(dispStat, 0);
		  spdlog::get("std")->debug("VBlank {:X}", dispStat);
		  SetHalf(DISPSTAT, dispStat);
		  if (BIT_RANGE(dispStat, 3, 3)) {
			spdlog::get("std")->debug("VBlank IntReq");
			auto intReq = GetHalf(IF);
			BIT_SET(intReq, 0);
			SetHalf(IF, intReq);
		  }
		} else {
		  state = Visible;
		}
	  }
	  break;
	}
	case VBlank: {
	  if (tickCount > CYCLES_PER_LINE) {
		tickCount = tickCount - CYCLES_PER_LINE;

		auto vCount = GetHalf(VCOUNT);
		vCount++;
		SetHalf(VCOUNT, vCount);

		if (vCount == 227) {
		  // Set VBlank flag to 0
		  auto dispStat = GetHalf(DISPSTAT);
		  BIT_CLEAR(dispStat, 0);
		  SetHalf(DISPSTAT, dispStat);
		}
		if (vCount >= TOTAL_LINES) {
		  SetHalf(VCOUNT, 0);
		  spdlog::get("std")->debug("VCount cleared");
		  state = Visible;
		}
	  }
	  break;
	}
  }
}

std::uint32_t GetScreenAreaOffset(std::uint32_t mapX,
                                  std::uint32_t mapY,
                                  std::uint_fast8_t screenSize) {
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

std::optional<std::uint16_t> PPU::TilePixelAtAbsoluteBGPosition(
    const BGControlInfo& bgCnt,
    const std::uint16_t& x,
    const std::uint16_t& y) {
  // Get tile coords
  auto mapX = x / TILE_PIXEL_WIDTH;
  auto mapY = y / TILE_PIXEL_HEIGHT;
  auto mapIndex =
      (mapX % TILE_AREA_WIDTH) + ((mapY % TILE_AREA_HEIGHT) * TILE_AREA_WIDTH);
  auto pixelX = x % TILE_PIXEL_WIDTH;
  auto pixelY = y % TILE_PIXEL_HEIGHT;

  auto screenAreaAddressInc = GetScreenAreaOffset(mapX, mapY, bgCnt.screenSize);
  // Parse tile data
  auto bgMapEntry = GetHalf(bgCnt.mapDataBase + screenAreaAddressInc +
                            (mapIndex * BYTES_PER_ENTRY));
  auto tileNumber = BIT_RANGE(bgMapEntry, 0, 9);
  bool horizontalFlip = BIT_RANGE(bgMapEntry, 10, 10);
  bool verticalFlip = BIT_RANGE(bgMapEntry, 11, 11);
  auto paletteNumber = BIT_RANGE(bgMapEntry, 12, 15);

  return GetTilePixel(tileNumber, pixelX, pixelY, bgCnt.colorDepth,
                      bgCnt.tileDataBase, verticalFlip, horizontalFlip,
                      paletteNumber);
}

std::optional<std::uint16_t> PPU::GetTilePixel(std::uint16_t tileNumber,
                                               std::uint16_t x,
                                               std::uint16_t y,
                                               std::uint16_t colorDepth,
                                               std::uint32_t tileDataBase,
                                               bool verticalFlip,
                                               bool horizontalFlip,
                                               std::uint16_t paletteNumber) {
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
  auto pixelPalette = GetByte(startOfTileAddress + positionInTile);

  if (colorDepth == 4) {
	if ((x % 2 == 0)) {
	  pixelPalette = BIT_RANGE(pixelPalette, 0, 3);
	} else {
	  pixelPalette = BIT_RANGE(pixelPalette, 4, 7);
	}
	if (pixelPalette == 0)
	  return {};
	return GetBgColorFromPalette(paletteNumber, pixelPalette);
  } else  // equal to 8
  {
	if (pixelPalette == 0)
	  return {};
	return GetBgColorFromPalette(pixelPalette);
  }
}

void PPU::TextBGLine(const std::uint32_t& BG_ID) {
  auto bgCnt = BGControlInfo(BG_ID, GetHalf(BGCNT[BG_ID]));
  auto bgXOffset = GetHalf(BGHOFS[BG_ID]) & NBIT_MASK(9);
  auto bgYOffset = GetHalf(BGVOFS[BG_ID]) & NBIT_MASK(9);

  auto y = GetHalf(VCOUNT);
  for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
	auto absoluteX = (x + bgXOffset) % TEXT_BGMAP_SIZES[bgCnt.screenSize][0];
	auto absoluteY = (y + bgYOffset) % TEXT_BGMAP_SIZES[bgCnt.screenSize][1];
	auto framebufferIndex = Screen::SCREEN_WIDTH * y + x;
	auto pixel = TilePixelAtAbsoluteBGPosition(bgCnt, absoluteX, absoluteY);
	if (pixel)
	  fb[framebufferIndex] = pixel.value();
  }
}

const std::uint32_t OAM_ENTRIES = 128;
void PPU::DrawObjects() {
  for (std::int32_t i = OAM_ENTRIES - 1; i >= 0; i--) {
	auto objAddress = OAM_START + (i * 8);
	auto objAttr0 = GetHalf(objAddress);
	auto drawObjectEnabled = BIT_RANGE(objAttr0, 8, 9) != 0b10;
	if (drawObjectEnabled) {
	  auto objAttr1 = GetHalf(objAddress + 2);
	  auto objAttr2 = GetHalf(objAddress + 4);
	  auto objAttrs = ObjAttributes(objAttr0, objAttr1, objAttr2);
	  DrawObject(objAttrs);
	}
  }
}

const uint16_t objDimensions[4][3][2] = {{{1, 1}, {2, 1}, {1, 2}},
                                         {{2, 2}, {4, 1}, {1, 4}},
                                         {{4, 4}, {4, 2}, {2, 4}},
                                         {{8, 8}, {8, 4}, {4, 8}}};

void PPU::DrawObject(ObjAttributes objAttrs) {
  auto [spriteWidth, spriteHeight] =
      objDimensions[objAttrs.attr1.b.objSize][objAttrs.attr0.b.objShape];
  auto topLeftTile = objAttrs.attr2.b.characterName;
  auto characterMapping = BIT_RANGE(GetHalf(DISPCNT), 6, 6);
  auto halfTiles = objAttrs.attr0.b.colorsPalettes ? 2u : 1u;
  auto colorDepth = objAttrs.attr0.b.colorsPalettes ? 8u : 4u;
  auto tileYIncrement = characterMapping ? (halfTiles * spriteWidth) : 0x20;

  for (auto tileX = 0; tileX < spriteWidth; tileX++) {
	for (auto tileY = 0; tileY < spriteHeight; tileY++) {
	  auto tileNumber =
	      topLeftTile + (tileX * halfTiles) + (tileY * tileYIncrement);

	  if (colorDepth == 8)
		tileNumber /= 2;

	  auto actualTileX = tileX;
	  if (objAttrs.attr1.b.horizontalFlip)
		actualTileX = spriteWidth - tileX - 1;

	  auto actualTileY = tileY;
	  if (objAttrs.attr1.b.verticalFlip)
		actualTileY = spriteHeight - tileY - 1;

	  auto x = objAttrs.attr1.b.xCoord + TILE_PIXEL_WIDTH * actualTileX;
	  auto y = objAttrs.attr0.b.yCoord + TILE_PIXEL_HEIGHT * actualTileY;

	  DrawTile(x, y, tileNumber, colorDepth, OBJ_START_ADDRESS,
	           objAttrs.attr1.b.verticalFlip, objAttrs.attr1.b.horizontalFlip,
	           objAttrs.attr2.b.paletteNumber);
	}
  }
}

void PPU::DrawTile(std::uint16_t startX,
                   std::uint16_t startY,
                   std::uint16_t tileNumber,
                   std::uint16_t colorDepth,
                   std::uint32_t tileDataBase,
                   bool verticalFlip,
                   bool horizontalFlip,
                   std::uint16_t paletteNumber) {
  for (auto x = 0u; x < TILE_PIXEL_WIDTH; x++) {
	for (auto y = 0u; y < TILE_PIXEL_HEIGHT; y++) {
	  auto totalX = (x + startX);
	  auto totalY = (y + startY);
	  if (totalX < Screen::SCREEN_WIDTH && totalY < Screen::SCREEN_HEIGHT) {
		auto pixel = GetTilePixel(tileNumber, x, y, colorDepth, tileDataBase,
		                          verticalFlip, horizontalFlip, paletteNumber);
		if (pixel) {
		  auto fbPos = totalX + (Screen::SCREEN_WIDTH * totalY);
		  fb[fbPos] = pixel.value();
		}
	  }
	}
  }
}
uint8_t PPU::GetLayerPriority(uint8_t layer) {
  auto bgCnt = GetHalf(BGCNT[layer]);
  auto bgPriority = BIT_RANGE(bgCnt, 0, 1);
  return bgPriority;
}

std::vector<uint8_t> PPU::GetBGDrawOrder(std::vector<uint8_t> layers,
                                         uint8_t screenDisplay) {
  // Find active layers
  std::vector<uint8_t> activeLayers = {};
  for (const auto& layer : layers) {
	if (BIT_RANGE(screenDisplay, layer, layer)) {
	  activeLayers.push_back(layer);
	}
  }
  // Order active layers
  std::map<uint8_t, uint8_t> orderedLayers;
  for (const auto& layer : activeLayers) {
	const uint8_t NUM_BGS = 4;
	auto layerScore = layer + GetLayerPriority(layer) * NUM_BGS;
	orderedLayers[layerScore] = layer;
  }

  std::vector<uint8_t> resultLayers = {};
  for (const auto& pair : orderedLayers) {
	resultLayers.push_back(pair.second);
  }

  return resultLayers;
}

void PPU::DrawLine() {
  auto vCount = GetHalf(VCOUNT);
  auto dispCnt = GetHalf(DISPCNT);
  auto screenDisplay = BIT_RANGE(dispCnt, 8, 11);
  auto bgMode = BIT_RANGE(dispCnt, 0, 2);
  auto frame = BIT_RANGE(dispCnt, 3, 3);

  switch (bgMode) {
	case 0: {
	  auto bgOrder = GetBGDrawOrder({0, 1, 2, 3}, screenDisplay);
	  for (auto bg = bgOrder.rbegin(); bg != bgOrder.rend(); ++bg) {
		TextBGLine(*bg);
	  }
	  break;
	}
	case 1:
	  TextBGLine(0);
	  break;
	case 2:
	  TextBGLine(2);
	  break;
	case 3: {
	  for (std::uint16_t pixel = (vCount * Screen::SCREEN_WIDTH);
	       pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
		fb[pixel] = GetHalf(VRAM_START + pixel * 2);
	  }
	} break;
	case 4:
	case 5: {
	  for (std::uint16_t pixel = (vCount * Screen::SCREEN_WIDTH);
	       pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
		auto colorID = GetByte(VRAM_START + (frame * 0xA000) + pixel);
		fb[pixel] = GetBgColorFromPalette(colorID);
	  }
	  // TODO: Actually implement Mode 5
	} break;
	default:
	  spdlog::get("std")->error("Unsupported bgMode");
	  break;
  }
  return;
}

std::uint16_t PPU::GetBgColorFromPalette(const std::uint32_t& paletteNumber,
                                         const std::uint32_t& colorID) {
  return GetBgColorFromPalette(paletteNumber * 16u + colorID);
}

std::uint16_t PPU::GetBgColorFromPalette(const std::uint32_t& colorID) {
  return GetHalf(colorID * 2 + PRAM_START);
}

std::uint8_t PPU::GetByte(const std::uint32_t& address) {
  return memory->Read(AccessSize::Byte, address, Sequentiality::FREE);
}

std::uint16_t PPU::GetHalf(const std::uint32_t& address) {
  return memory->Read(AccessSize::Half, address, Sequentiality::FREE);
}

void PPU::SetHalf(const std::uint32_t& address, const std::uint16_t& value) {
  return memory->Write(AccessSize::Half, address, value, Sequentiality::FREE);
}