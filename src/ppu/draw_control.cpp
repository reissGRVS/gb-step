#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "ppu/pixel.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>

void PPU::MergeRows(std::vector<uint8_t>& bgOrder)
{
	const U16 fbIndex = GET_HALF(VCOUNT) * Screen::SCREEN_WIDTH;

	for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
		auto pos = fbIndex + x;

		OptPixel firstPrioPixel = {};
		OptPixel secondPrioPixel = {};
		bool applyEffects = false;
		
		//Find highest priority pixel, and second if alphablending is enabled
		for (const auto& bg : bgOrder) {
			if (rows[bg][x].has_value()) {
				if (!firstPrioPixel.has_value())
				{
					firstPrioPixel = rows[bg][x];
					depth[pos] = GetLayerPriority(bg);
					secondtarget[pos] = bldCnt.secondTarget[bg];
					applyEffects = bldCnt.firstTarget[bg];
				}
				else if (bldCnt.colorSpecialEffect == BldCnt::AlphaBlending)
				{
					//If next found pixel is a blend target keep track of it
					if (bldCnt.secondTarget[bg])
						secondPrioPixel = rows[bg][x];
					break;
				}
			}
		}


		if (firstPrioPixel.has_value())
		{	
			if (!applyEffects || bldCnt.colorSpecialEffect == BldCnt::None)
			{
				fb[pos] = firstPrioPixel.value();
			}
			else
			{
				SetSFXPixel(firstPrioPixel, secondPrioPixel, fb[pos]);
			}
		}

	}
}


void PPU::SetSFXPixel(OptPixel& firstPrioPixel, OptPixel& secondPrioPixel, U16& dest)
{
	if (!firstPrioPixel.has_value())
		return;

	Pixel firstPixel{firstPrioPixel.value()};

	switch (bldCnt.colorSpecialEffect)
	{
		case BldCnt::None:
		{
			break;
		}
		case BldCnt::AlphaBlending:
		{
			//Blend with backdrop if possible
			if (!secondPrioPixel.has_value() && bldCnt.secondTarget[5])
				secondPrioPixel = dest;

			if (secondPrioPixel.has_value())
			{
				Pixel secondPixel{secondPrioPixel.value()};
				firstPixel.Blend(secondPixel, eva, evb);
			}
			break;
		}
		case BldCnt::BrightnessIncrease:
		{
			firstPixel.BrightnessIncrease(evy);
			break;
		}
		case BldCnt::BrightnessDecrease:
		{
			firstPixel.BrightnessDecrease(evy);
			break;
		}
	}

	dest = firstPixel.Value();
}

void PPU::DrawLine()
{
	auto vCount = GET_HALF(VCOUNT);
	auto dispCnt = GET_HALF(DISPCNT);
	auto screenDisplay = BIT_RANGE(dispCnt, 8, 11);
	auto bgMode = BIT_RANGE(dispCnt, 0, 2);
	auto frame = BIT_RANGE(dispCnt, 4, 4);
	rowsFilled.fill(0);

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