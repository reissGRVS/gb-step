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

		fetchScanline();
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

void PPU::BGLine(const uint32_t& BGCNT) {
  auto bgCnt = getHalf(BGCNT);
  auto bgTileBase = VRAM_START + BIT_RANGE(bgCnt, 2, 3) * 0x4000u;
  auto colorDepth = BIT_RANGE(bgCnt, 7, 7) ? 8u : 4u;
  auto pixelsPerByte = 8u / colorDepth;
  auto bgMapBase = VRAM_START + BIT_RANGE(bgCnt, 8, 12) * 0x800u;

  std::uint32_t PIXELS_PER_TILE_LINE = 8;
  std::uint32_t TILE_DATA_ROWS = 8;

  auto y = getHalf(VCOUNT);
  auto mapY = y / 8;
  auto tileY = y % 8;
  for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
	auto mapX = x / PIXELS_PER_TILE_LINE;
	auto tileX = x % PIXELS_PER_TILE_LINE;

	auto mapPos = mapX + (mapY * 32);
	auto tileData = getHalf(bgMapBase + (mapPos * 2));
	auto tileNumber = tileData & NBIT_MASK(10);
	// auto paletteNumber = tileData >> 12;
	auto bytesPerTile = colorDepth * TILE_DATA_ROWS;

	auto pixelPalette = getByte(bgTileBase + (tileNumber * bytesPerTile) +
	                            (tileX / pixelsPerByte) + (tileY * colorDepth));

	auto fbPos = x + y * Screen::SCREEN_WIDTH;
	if (colorDepth == 4) {
	  // Process to pixels at a time
	  auto pixelColorL = getBgColorFromPallete(pixelPalette & NBIT_MASK(4));
	  auto pixelColorR = getBgColorFromPallete(pixelPalette >> 4);
	  fb[fbPos] = pixelColorL;
	  fb[fbPos + 1] = pixelColorR;
	  x++;
	} else {
	  auto pixelColor = getBgColorFromPallete(pixelPalette);
	  fb[fbPos] = pixelColor;
	}
  }
  // TODO: HFlip VFlip
}

void PPU::fetchScanline() {
  auto vCount = getHalf(VCOUNT);
  auto dispCnt = getHalf(DISPCNT);
  auto bgMode = BIT_RANGE(dispCnt, 0, 2);
  auto frame = BIT_RANGE(dispCnt, 3, 3);

  for (std::uint16_t pixel = (vCount * Screen::SCREEN_WIDTH);
       pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
	switch (bgMode) {
	  case 0:
		// TODO: This is dumb, get rid of this, temp solution
		BGLine(BG0CNT);
		return;
	  case 1:
		BGLine(BG0CNT);
		return;
	  case 2:
		BGLine(BG2CNT);
		return;
	  case 3: {
		fb[pixel] = getHalf(VRAM_START + pixel * 2);
	  } break;
	  case 4:
	  case 5: {
		auto colorID = getByte(VRAM_START + (frame * 0xA000) + pixel);
		fb[pixel] = getBgColorFromPallete(colorID);
	  } break;
	  default:
		spdlog::get("std")->error("Unsupported bgMode");
		break;
		// TODO: complain
	}
  }
  return;
}
std::uint16_t PPU::getBgColorFromPallete(const std::uint32_t& colorID) {
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