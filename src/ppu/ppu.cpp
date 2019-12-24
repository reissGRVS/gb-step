#include "ppu/ppu.hpp"
#include "memory/regions.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>

const U32 CYCLES_PER_VISIBLE = 960, CYCLES_PER_HBLANK = 272,
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

void PPU::Execute(U32 ticks)
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

U16 PPU::GetDispStat(U8 bit)
{
	auto dispStat = Read(Half, DISPSTAT, FREE);
	if (bit == VCountSetting) {
		return BIT_RANGE(dispStat, 8, 15);
	} else {
		return BIT_RANGE(dispStat, bit, bit);
	}
}

void PPU::UpdateDispStat(U8 bit, bool set)
{
	auto dispStat = Read(Half, DISPSTAT, FREE);
	if (set)
		BIT_SET(dispStat, bit);
	else
		BIT_CLEAR(dispStat, bit);
	Write(Half, DISPSTAT, dispStat, FREE);
}

U16 PPU::IncrementVCount()
{
	auto vCount = Read(Half, VCOUNT, FREE);
	vCount++;
	Write(Half, VCOUNT, vCount, FREE);

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