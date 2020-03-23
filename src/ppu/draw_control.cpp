#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>


const Window& PPU::GetActiveWindow(U16 x, U16 y)
{
	if (windows[WindowID::Win0].InRange(x,y)) return windows[WindowID::Win0];
	if (windows[WindowID::Win1].InRange(x,y)) return windows[WindowID::Win1];

	auto fbIndex = (y * Screen::SCREEN_WIDTH + x);
	if (objFb[fbIndex].mask) return windows[WindowID::Obj];

	return windows[WindowID::Outside];
}

void PPU::MergeRows(std::vector<uint8_t>& bgOrder)
{
	const auto y = GET_HALF(VCOUNT);
	const U16 fbIndex = y * Screen::SCREEN_WIDTH;

	for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
		auto pos = fbIndex + x;
		
		const Window& activeWindow = GetActiveWindow(x, y);
		
		OptPixel firstPrioPixel = {};
		U16 firstPrio = 5;
		OptPixel secondPrioPixel = {};
		U16 secondPrio = 5;
		bool applyEffects = false;
		
		//Find highest priority background pixel, and second if alphablending is enabled
		for (const auto& bg : bgOrder) {
			if (!activeWindow.bgEnable[bg]) continue;

			if (rows[bg][x].has_value()) {
				if (!firstPrioPixel.has_value())
				{
					firstPrio = GetLayerPriority(bg);
					firstPrioPixel = rows[bg][x];
					applyEffects = bldCnt.firstTarget[bg];
				}
				else if (bldCnt.colorSpecialEffect == BldCnt::AlphaBlending)
				{
					//If next found pixel is a blend target keep track of it
					if (bldCnt.secondTarget[bg])
					{
						secondPrioPixel = rows[bg][x];
						secondPrio = GetLayerPriority(bg);
					}
					break;
				}
			}
		}

		bool forceBlend = false;

		//Check if obj is higher priority than selected layers
		if (activeWindow.objEnable)
		{
			ObjPixel objPixel = objFb[pos];
			
			if (objPixel.prio <= firstPrio)
			{
				applyEffects = bldCnt.firstTarget[BldCnt::TargetLayer::Sprites];
				forceBlend = objPixel.transparency;
				if (forceBlend || bldCnt.colorSpecialEffect == BldCnt::AlphaBlending)
				{
					secondPrioPixel = firstPrioPixel;
				}
				firstPrioPixel = objPixel.pixel;
			}
			else if (objPixel.prio <= secondPrio)
			{
				if (bldCnt.secondTarget[BldCnt::TargetLayer::Sprites])
					secondPrioPixel = objPixel.pixel;
				else
					secondPrioPixel = {};
			}
		}

		if (!activeWindow.sfxEnable) applyEffects = false; //TODO: should this override forceblend?

		if (firstPrioPixel.has_value())
		{	
			if (forceBlend)
			{
				SetSFXPixel(firstPrioPixel, secondPrioPixel, fb[pos], BldCnt::AlphaBlending);
			}
			else if (!applyEffects || bldCnt.colorSpecialEffect == BldCnt::None)
			{
				fb[pos] = firstPrioPixel.value();
			}
			else
			{
				SetSFXPixel(firstPrioPixel, secondPrioPixel, fb[pos], bldCnt.colorSpecialEffect);
			}
		}

	}
}

void PPU::DrawLine()
{
	auto vCount = GET_HALF(VCOUNT);
	dispCnt = DispCnt(GET_HALF(DISPCNT));
	auto screenDisplay = dispCnt.screenDisplay;
	auto bgMode = dispCnt.bgMode;
	auto frame = dispCnt.frameSelect;
	for (auto& row : rows)
	{
		row.fill({});
	}

	switch (bgMode) {
	case 0: {
		auto bgOrder = GetBGDrawOrder({ 0, 1, 2, 3 }, screenDisplay);
		for (const auto& bg : bgOrder) {
			TextBGLine(bg);
		}
		MergeRows(bgOrder);
	} break;
	case 1: {
		auto bgOrder = GetBGDrawOrder({ 0, 1, 2}, screenDisplay);
		for (const auto& bg : bgOrder) {
			if (bg == 2)
			{
				RotScaleBGLine(bg);
			}
			else
			{	
				TextBGLine(bg);
			}
		}
		MergeRows(bgOrder);
	} break;
	case 2:
	{
		auto bgOrder = GetBGDrawOrder({ 2, 3 }, screenDisplay);
		for (const auto& bg : bgOrder) {
			RotScaleBGLine(bg);
		}
		MergeRows(bgOrder);
	} break;
	case 3: {
		for (U16 pixel = (vCount * Screen::SCREEN_WIDTH);
			 pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
			fb[pixel] = memory->GetHalf(VRAM_START + pixel * 2);
		}
	} break;
	case 4:
	case 5: {
		for (U16 pixel = (vCount * Screen::SCREEN_WIDTH);
			 pixel < ((vCount + 1) * Screen::SCREEN_WIDTH); pixel++) {
			auto colorID = memory->GetByte(VRAM_START + (frame * 0xA000) + pixel);
			fb[pixel] = GetBgColorFromPalette(colorID);
		}
		// TODO: Actually implement Mode 5
	} break;
	default:
		
		break;
	}
	return;
}

uint8_t PPU::GetLayerPriority(uint8_t layer)
{
	auto bgCnt = GET_HALF(BGCNT[layer]);
	auto bgPriority = BIT_RANGE(bgCnt, 0, 1);
	return bgPriority;
}

std::vector<uint8_t> PPU::GetBGDrawOrder(std::vector<uint8_t> layers,
	uint8_t screenDisplay)
{
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