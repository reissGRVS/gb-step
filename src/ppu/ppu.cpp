#include "ppu/ppu.hpp"
#include "memory/regions.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>

const U32 CYCLES_PER_VISIBLE = 960, CYCLES_PER_HBLANK = 272,
		  CYCLES_PER_LINE = CYCLES_PER_VISIBLE + CYCLES_PER_HBLANK,
		  VISIBLE_LINES = 160, VBLANK_LINES = 68,
		  TOTAL_LINES = VISIBLE_LINES + VBLANK_LINES;

const U32 BGX[2] = { BG2X, BG3X };
const U32 BGY[2] = { BG2Y, BG3Y };

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
	secondtarget.fill(false);
	

	// Set VBlank flag and Request Interrupt
	UpdateDispStat(VBlankFlag, true);

	if (GetDispStat(VBlankIRQEnable)) {
		
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
		//Reload RotScale registers
		{
			for (auto bgId = 2; bgId <= 3; bgId++)
			{
				auto bgX = memory->GetWord(BGX[bgId-2]);
				bgXRef[bgId-2] = bgX;
				if (BIT_RANGE(bgX, 27, 27)) bgXRef[bgId-2] |= 0xF0000000;
				auto bgY = memory->GetWord(BGY[bgId-2]);
				bgYRef[bgId-2] = bgY;
				if (BIT_RANGE(bgY, 27, 27)) bgYRef[bgId-2] |= 0xF0000000;
			}
			bldCnt = BldCnt{GET_HALF(BLDCNT)};
			U16 bldAlpha = GET_HALF(BLDALPHA);
			eva = BIT_RANGE(bldAlpha, 0, 4);
			if (eva > 16) {eva = 16;}
			evb = BIT_RANGE(bldAlpha, 8, 12);
			if (evb > 16) {evb = 16;}
			U16 bldY = GET_HALF(BLDY);
			evy = BIT_RANGE(bldY, 0, 4);
			if (evy > 16) {evy = 16;}
		}


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
			
			memory->RequestInterrupt(Interrupt::VCounter);
		}
	} else {
		UpdateDispStat(VCounterFlag, false);
	}

	return vCount;
}