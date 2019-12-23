#pragma once

#include "memory/regions.hpp"
#include "utils.hpp"
#include <cstdint>

const std::uint32_t BG_TILE_DATA_UNITSIZE = 0x4000u;
const std::uint32_t BG_MAP_DATA_UNITSIZE = 0x800u;

struct BGControlInfo {
	BGControlInfo(const std::uint_fast8_t& bgID, const uint32_t& bgCnt)
		: ID(bgID)
		, priority(BIT_RANGE(bgCnt, 0, 1))
		, tileDataBase(VRAM_START + BIT_RANGE(bgCnt, 2, 3) * BG_TILE_DATA_UNITSIZE)
		, mosaic(BIT_RANGE(bgCnt, 6, 6))
		, colorsPalettes(BIT_RANGE(bgCnt, 7, 7))
		, colorDepth(colorsPalettes ? 8u : 4u)
		, mapDataBase(VRAM_START + BIT_RANGE(bgCnt, 8, 12) * BG_MAP_DATA_UNITSIZE)
		, wrapAround(BIT_RANGE(bgCnt, 13, 13) || ID == 0 || ID == 1)
		, screenSize(BIT_RANGE(bgCnt, 14, 15)){};

	const std::uint_fast8_t ID;

	const std::uint_fast8_t priority;
	const std::uint32_t tileDataBase;
	const bool mosaic;
	const uint_fast8_t colorsPalettes;
	const uint_fast8_t colorDepth;

	const std::uint32_t mapDataBase;
	const bool wrapAround;
	const uint_fast8_t screenSize;
};