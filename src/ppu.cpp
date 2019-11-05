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

		// TODO: Add line to framebuffer
		fetchScanline();
		// Set HBlank flag and Request Interrupt
		auto dispStat = getHalf(DISPSTAT);
		BIT_SET(dispStat, 1);
		setHalf(DISPSTAT, dispStat);

		auto intReq = getHalf(IF);
		BIT_SET(intReq, 1);
		setHalf(DISPSTAT, intReq);
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

		if (vCount >= VISIBLE_LINES) {
		  state = VBlank;
		  auto dispCnt = getHalf(DISPCNT);
		  spdlog::info("VBlank {:X}", dispCnt);
		  screen.render(fb);

		  // Set VBlank flag and Request Interrupt
		  auto dispStat = getHalf(DISPSTAT);
		  BIT_SET(dispStat, 0);
		  setHalf(DISPSTAT, dispStat);
		  auto intReq = getHalf(IF);
		  BIT_SET(intReq, 0);
		  setHalf(DISPSTAT, intReq);
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
		  state = Visible;
		}
	  }
	  break;
	}
  }
}

void PPU::fetchScanline() {
  auto vCount = getHalf(VCOUNT);
  auto bgMode = 3;

  switch (bgMode) {
	case 3: {
	  for (std::uint16_t pixel = (vCount * Screen::SCREEN_WIDTH);
	       pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
		fb[pixel] = getHalf(vram_start + pixel);
	  }
	  break;
	}
	default:
	  spdlog::error("Unsupported bgMode");
	  break;
	  // TODO: complain
  }
  return;
}

std::uint16_t PPU::getHalf(const std::uint32_t& address) {
  return memory->Read(Memory::AccessSize::Half, address,
                      Memory::Sequentiality::PPU);
}

void PPU::setHalf(const std::uint32_t& address, const std::uint16_t& value) {
  return memory->Write(Memory::AccessSize::Half, address, value,
                       Memory::Sequentiality::PPU);
}