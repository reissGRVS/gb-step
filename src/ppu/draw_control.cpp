#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <map>

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
		for (auto bg : bgOrder) {
			TextBGLine(bg);
		}
		break;
	}
	case 1: {
		auto bgOrder = GetBGDrawOrder({ 0, 1, 2 }, screenDisplay);
		for (auto bg : bgOrder) {
			TextBGLine(bg);
		}
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
		spdlog::get("std")->error("Unsupported bgMode");
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