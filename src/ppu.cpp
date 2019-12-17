#include "ppu.hpp"
#include "memory_regions.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

void PPU::Execute(std::uint32_t ticks) {
  tickCount += ticks;

  // https://problemkaputt.de/gbatek.htm#lcddimensionsandtimings
  switch (state) {
	case Visible: {
	  if (tickCount > CYCLES_PER_VISIBLE) {
		tickCount = tickCount - CYCLES_PER_VISIBLE;
		state = HBlank;

		drawLine();
		// Set HBlank flag and Request Interrupt
		auto dispStat = getHalf(DISPSTAT);
		BIT_SET(dispStat, 1);
		setHalf(DISPSTAT, dispStat);
		if (BIT_RANGE(dispStat, 4, 4)) {
		  spdlog::get("std")->debug("HBlank IntReq");
		  auto intReq = getHalf(IF);
		  BIT_SET(intReq, 1);
		  setHalf(IF, intReq);
		}
	  }
	  break;
	}
	case HBlank: {
	  if (tickCount > CYCLES_PER_HBLANK) {
		tickCount = tickCount - CYCLES_PER_HBLANK;

		// Set HBlank flag to 0
		auto dispStat = getHalf(DISPSTAT);
		BIT_CLEAR(dispStat, 1);
		setHalf(DISPSTAT, dispStat);

		auto vCount = getHalf(VCOUNT);
		vCount++;
		setHalf(VCOUNT, vCount);
		spdlog::get("std")->debug("HBlank Line {}", vCount);

		if (vCount >= VISIBLE_LINES) {
		  state = VBlank;

		  screen.render(fb);

		  // Set VBlank flag and Request Interrupt
		  BIT_SET(dispStat, 0);
		  spdlog::get("std")->debug("VBlank {:X}", dispStat);
		  setHalf(DISPSTAT, dispStat);
		  if (BIT_RANGE(dispStat, 3, 3)) {
			spdlog::get("std")->debug("VBlank IntReq");
			auto intReq = getHalf(IF);
			BIT_SET(intReq, 0);
			setHalf(IF, intReq);
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

		auto vCount = getHalf(VCOUNT);
		vCount++;
		setHalf(VCOUNT, vCount);

		if (vCount == 227) {
		  // Set VBlank flag to 0
		  auto dispStat = getHalf(DISPSTAT);
		  BIT_CLEAR(dispStat, 0);
		  setHalf(DISPSTAT, dispStat);
		}
		if (vCount >= TOTAL_LINES) {
		  setHalf(VCOUNT, 0);
		  spdlog::get("std")->debug("VCount cleared");
		  state = Visible;
		}
	  }
	  break;
	}
  }
}

std::uint16_t PPU::TilePixelAtAbsoluteBGPosition(const BGControlInfo& bgCnt,
                                                 const std::uint16_t& x,
                                                 const std::uint16_t& y) {
  const std::uint32_t TILE_PIXEL_HEIGHT = 8, TILE_PIXEL_WIDTH = 8,
                      TILE_AREA_WIDTH = 32, BYTES_PER_ENTRY = 2;

  // Get tile coords
  auto mapY = y / TILE_PIXEL_HEIGHT;
  auto mapX = x / TILE_PIXEL_WIDTH;
  auto mapIndex = mapX + (mapY * TILE_AREA_WIDTH);
  auto tileY = y % TILE_PIXEL_HEIGHT;
  auto tileX = x % TILE_PIXEL_WIDTH;

  // Parse tile data
  auto bgMapEntry = getHalf(bgCnt.mapDataBase + (mapIndex * BYTES_PER_ENTRY));
  auto tileNumber = BIT_RANGE(bgMapEntry, 0, 9);
  //   auto horizontalFlip = BIT_RANGE(bgMapEntry, 10, 10);
  //   auto verticalFlip = BIT_RANGE(bgMapEntry, 11, 11);
  auto paletteNumber = BIT_RANGE(bgMapEntry, 12, 15);

  // Calculate position of tile pixel
  auto bytesPerTile = bgCnt.colorDepth * TILE_PIXEL_HEIGHT;
  auto startOfTileAddress = bgCnt.tileDataBase + (tileNumber * bytesPerTile);
  auto positionInTile =
      (tileX / bgCnt.pixelsPerByte) + (tileY * bgCnt.colorDepth);
  auto pixelPalette = getByte(startOfTileAddress + positionInTile);

  if (bgCnt.colorDepth == 4) {
	if (tileX % 2 == 0) {
	  pixelPalette = BIT_RANGE(pixelPalette, 0, 3);
	} else {
	  pixelPalette = BIT_RANGE(pixelPalette, 4, 7);
	}
	return getBgColorFromPalette(paletteNumber, pixelPalette);
  } else  // equal to 8
  {
	return getBgColorFromPalette(pixelPalette);
  }
}

void PPU::TextBGLine(const uint32_t& BG_ID) {
  auto bgCnt = BGControlInfo(BG_ID, getHalf(BGCNT[BG_ID]));
  auto y = getHalf(VCOUNT);
  for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
	auto fbPos = Screen::SCREEN_WIDTH * y + x;
	fb[fbPos] = TilePixelAtAbsoluteBGPosition(bgCnt, x, y);
  }
}

void PPU::drawLine() {
  auto vCount = getHalf(VCOUNT);
  auto dispCnt = getHalf(DISPCNT);
  auto bgMode = BIT_RANGE(dispCnt, 0, 2);
  auto frame = BIT_RANGE(dispCnt, 3, 3);

  switch (bgMode) {
	case 0:
	  // TODO: This is dumb, get rid of this, temp solution
	  TextBGLine(0);
	  break;
	case 1:
	  TextBGLine(0);
	  break;
	case 2:
	  TextBGLine(2);
	  break;
	case 3: {
	  for (std::uint16_t pixel = (vCount * Screen::SCREEN_WIDTH);
	       pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
		fb[pixel] = getHalf(VRAM_START + pixel * 2);
	  }
	} break;
	case 4:
	case 5: {
	  for (std::uint16_t pixel = (vCount * Screen::SCREEN_WIDTH);
	       pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
		auto colorID = getByte(VRAM_START + (frame * 0xA000) + pixel);
		fb[pixel] = getBgColorFromPalette(colorID);
	  }
	  // TODO: Actually implement Mode 5
	} break;
	default:
	  spdlog::get("std")->error("Unsupported bgMode");
	  break;
	  // TODO: complain
  }
  return;
}

std::uint16_t PPU::getBgColorFromPalette(const std::uint32_t& paletteNumber,
                                         const std::uint32_t& colorID) {
  return getBgColorFromPalette(paletteNumber * 16u + colorID);
}

std::uint16_t PPU::getBgColorFromPalette(const std::uint32_t& colorID) {
  return getHalf(colorID * 2 + PRAM_START);
}

std::uint8_t PPU::getByte(const std::uint32_t& address) {
  return memory->Read(AccessSize::Byte, address, Sequentiality::FREE);
}

std::uint16_t PPU::getHalf(const std::uint32_t& address) {
  return memory->Read(AccessSize::Half, address, Sequentiality::FREE);
}

void PPU::setHalf(const std::uint32_t& address, const std::uint16_t& value) {
  return memory->Write(AccessSize::Half, address, value, Sequentiality::FREE);
}