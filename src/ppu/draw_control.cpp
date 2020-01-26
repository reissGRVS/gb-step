#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "ppu/pixel.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>


struct BldCnt {
	BldCnt(U16 value)
	{
		for (U8 i = 0; i < firstTarget.size(); i++)
		{
			firstTarget[i] = BIT_RANGE(value, i, i);
			secondTarget[i] = BIT_RANGE(value, (i+8), (i+8));
		}
		colorSpecialEffect = (ColorSpecialEffect) BIT_RANGE(value, 6, 7);
	}

	std::array<bool, 6> firstTarget = {};
	
	enum ColorSpecialEffect {
		None = 0,
		AlphaBlending = 1,
		BrightnessIncrease = 2,
		BrightnessDecrease = 3
	};
	ColorSpecialEffect colorSpecialEffect;

	std::array<bool, 6> secondTarget = {};
};


void PPU::MergeRows(std::vector<uint8_t>& bgOrder)
{
	const auto vCount = GET_HALF(VCOUNT);
	U16 fbIndex = vCount * Screen::SCREEN_WIDTH;
	BldCnt bldCnt{GET_HALF(BLDCNT)};

	U16 bldAlpha = GET_HALF(BLDALPHA);
	U8 eva = BIT_RANGE(bldAlpha, 0, 4);
	if (eva > 16) {eva = 16;}
	U8 evb = BIT_RANGE(bldAlpha, 8, 12);
	if (evb > 16) {evb = 16;}
	U16 bldY = GET_HALF(BLDY);
	U8 evy = BIT_RANGE(bldY, 0, 4);
	if (evy > 16) {evy = 16;}

	auto backdrop = memory->GetHalf(PRAM_START);

	for (auto x = 0u; x < Screen::SCREEN_WIDTH; x++) {
		OptPixel firstPrioPixel = {};
		bool firstPrioPixelFound = false;
		U8 firstPrio = -1;
		OptPixel secondPrioPixel = {};
		bool secondPrioPixelFound = false;
		bool blend = false;
		for (const auto& bg : bgOrder) {
			if (!firstPrioPixelFound)
			{
				if (rows[bg][x].has_value())
				{
					firstPrioPixel = rows[bg][x];
					firstPrio = GetLayerPriority(bg);
					firstPrioPixelFound = true;
					if (!bldCnt.firstTarget[bg] || bldCnt.colorSpecialEffect != BldCnt::AlphaBlending)
					{
						break;
					}
					else
					{
						blend = true;
					}
				}
			}
			else
			{
				if (rows[bg][x].has_value())
				{
					secondPrioPixel = rows[bg][x];
					secondPrioPixelFound = true;
					if (!bldCnt.secondTarget[bg])
					{
						blend = false;
					}
					break;
				}
			}	
		}

		if (firstPrioPixel.has_value())
		{
			depth[fbIndex+x] = firstPrio;
			switch (bldCnt.colorSpecialEffect)
			{
			case BldCnt::None:
			{
				fb[fbIndex+x] = firstPrioPixel.value();
				break;
			}
			case BldCnt::AlphaBlending:
			{
				if (blend && !secondPrioPixelFound)
				{
					if (bldCnt.secondTarget[5])
					{
						secondPrioPixel = backdrop;
					}
					else
					{
						blend = false;
					}
				}

				if (blend)
				{
					Pixel firstPixel{firstPrioPixel.value()};
					Pixel secondPixel{secondPrioPixel.value()};
					firstPixel.Blend(secondPixel, eva, evb);
					fb[fbIndex+x] = firstPixel.Value();
				}
				else
				{
					fb[fbIndex+x] = firstPrioPixel.value();
				}
				break;
			}
			case BldCnt::BrightnessIncrease:
			{
				Pixel firstPixel{firstPrioPixel.value()};
				firstPixel.BrightnessIncrease(evy);
				fb[fbIndex+x] = firstPixel.Value();
				break;
			}
			case BldCnt::BrightnessDecrease:
			{
				Pixel firstPixel{firstPrioPixel.value()};
				firstPixel.BrightnessDecrease(evy);
				fb[fbIndex+x] = firstPixel.Value();
				break;
			}
			}
		}

	}
}

void PPU::DrawLine()
{
	auto vCount = GET_HALF(VCOUNT);
	auto dispCnt = GET_HALF(DISPCNT);
	auto screenDisplay = BIT_RANGE(dispCnt, 8, 11);
	auto bgMode = BIT_RANGE(dispCnt, 0, 2);
	auto frame = BIT_RANGE(dispCnt, 4, 4);

	switch (bgMode) {
	case 0: {
		auto bgOrder = GetBGDrawOrder({ 0, 1, 2, 3 }, screenDisplay);
		for (const auto& bg : bgOrder) {
			TextBGLine(bg);
		}
		MergeRows(bgOrder);
		break;
	}
	case 1: {
		auto bgOrder = GetBGDrawOrder({ 0, 1}, screenDisplay);
		for (const auto& bg : bgOrder) {
			TextBGLine(bg);
		}
		MergeRows(bgOrder);
		break;
	}
	case 2:
		break;
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