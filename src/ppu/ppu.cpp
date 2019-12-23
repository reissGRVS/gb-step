#include "ppu/ppu.hpp"
#include "memory/regions.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>

const std::uint32_t CYCLES_PER_VISIBLE = 960, CYCLES_PER_HBLANK = 272,
					CYCLES_PER_LINE = CYCLES_PER_VISIBLE + CYCLES_PER_HBLANK,
					VISIBLE_LINES = 160, VBLANK_LINES = 68,
					TOTAL_LINES = VISIBLE_LINES + VBLANK_LINES,
					CYCLES_PER_VBLANK = CYCLES_PER_LINE * VBLANK_LINES;

enum DispStatInfo {
	VBlankFlag = 0,
	HBlankFlag = 1,
	VCounterFlag = 2,
	VBlankIRQEnable = 3,
	HBlankIRQEnable = 4,
	VCounterIRQEnable = 5,
	VCountSetting = 6
};

void PPU::Execute(std::uint32_t ticks)
{
	tickCount += ticks;

	// https://problemkaputt.de/gbatek.htm#lcddimensionsandtimings
	switch (state) {
	case Visible: {
		if (tickCount > CYCLES_PER_VISIBLE) {
			tickCount = tickCount - CYCLES_PER_VISIBLE;
			ToHBlank();
		}
		break;
	}
	case HBlank: {
		if (tickCount > CYCLES_PER_HBLANK) {
			tickCount = tickCount - CYCLES_PER_HBLANK;
			OnHBlankFinish();
		}
		break;
	}
	case VBlank: {
		if (tickCount > CYCLES_PER_LINE) {
			tickCount = tickCount - CYCLES_PER_LINE;
			OnVBlankLineFinish();
		}
		break;
	}
	}
}

void PPU::ToHBlank()
{
	state = HBlank;
	HBlankCallback(true);
	DrawLine();
	UpdateDispStat(HBlankFlag, true);

	if (GetDispStat(HBlankIRQEnable)) {
		spdlog::get("std")->debug("HBlank IntReq");
		memory->RequestInterrupt(Interrupt::HBlank);
	}
}

void PPU::OnHBlankFinish()
{
	HBlankCallback(false);
	UpdateDispStat(HBlankFlag, false);

	auto vCount = IncrementVCount();

	if (vCount >= VISIBLE_LINES) {
		ToVBlank();
	} else {
		state = Visible;
	}
}

void PPU::ToVBlank()
{
	state = VBlank;
	DrawObjects();
	screen.render(fb);
	fb.fill(memory->GetHalf(PRAM_START));
	depth.fill(MAX_DEPTH);

	// Set VBlank flag and Request Interrupt
	UpdateDispStat(VBlankFlag, true);

	if (GetDispStat(VBlankIRQEnable)) {
		spdlog::get("std")->debug("VBlank IntReq");
		memory->RequestInterrupt(Interrupt::VBlank);
	}
	VBlankCallback(true);
}

void PPU::OnVBlankLineFinish()
{
	auto vCount = IncrementVCount();

	if (vCount == 227) {
		UpdateDispStat(VBlankFlag, false);
	}
	if (vCount >= TOTAL_LINES) {
		spdlog::get("std")->debug("VBlank Finished");
		memory->SetHalf(VCOUNT, 0);
		VBlankCallback(false);
		state = Visible;
	}
}

std::uint16_t PPU::GetDispStat(std::uint8_t bit)
{
	auto dispStat = memory->GetHalf(DISPSTAT);
	if (bit == VCountSetting) {
		return BIT_RANGE(dispStat, 8, 15);
	} else {
		return BIT_RANGE(dispStat, bit, bit);
	}
}

void PPU::UpdateDispStat(std::uint8_t bit, bool set)
{
	auto dispStat = memory->GetHalf(DISPSTAT);
	if (set)
		BIT_SET(dispStat, bit);
	else
		BIT_CLEAR(dispStat, bit);
	memory->SetHalf(DISPSTAT, dispStat);
}

std::uint16_t PPU::IncrementVCount()
{
	auto vCount = memory->GetHalf(VCOUNT);
	vCount++;
	memory->SetHalf(VCOUNT, vCount);

	if (GetDispStat(VCountSetting) == vCount) {
		UpdateDispStat(VCounterFlag, true);
		if (GetDispStat(VCounterIRQEnable)) {
			spdlog::get("std")->debug("VCounter IntReq");
			memory->RequestInterrupt(Interrupt::VCounter);
		}
	} else {
		UpdateDispStat(VCounterFlag, false);
	}

	return vCount;
}