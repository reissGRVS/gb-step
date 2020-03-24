#pragma once

#include "memory/regions.hpp"
#include "utils.hpp"
#include "int.hpp"

const U32 BG_TILE_DATA_UNITSIZE = 0x4000u;
const U32 BG_MAP_DATA_UNITSIZE = 0x800u;

struct BGControlInfo {
	BGControlInfo(const U8& bgID, const uint32_t& bgCnt)
		: ID(bgID)
		, priority(BIT_RANGE(bgCnt, 0, 1))
		, tileDataBase(VRAM_START + BIT_RANGE(bgCnt, 2, 3) * BG_TILE_DATA_UNITSIZE)
		, mosaic(BIT_RANGE(bgCnt, 6, 6))
		, colorsPalettes(BIT_RANGE(bgCnt, 7, 7))
		, colorDepth(colorsPalettes ? 8u : 4u)
		, mapDataBase(VRAM_START + BIT_RANGE(bgCnt, 8, 12) * BG_MAP_DATA_UNITSIZE)
		, wrapAround(BIT_RANGE(bgCnt, 13, 13) || ID == 0 || ID == 1)
		, screenSize(BIT_RANGE(bgCnt, 14, 15)){};

	void UpdateValue(const uint32_t& bgCnt)
	{
		priority = BIT_RANGE(bgCnt, 0, 1);
		tileDataBase= VRAM_START + BIT_RANGE(bgCnt, 2, 3) * BG_TILE_DATA_UNITSIZE;
		mosaic = BIT_RANGE(bgCnt, 6, 6);
		colorsPalettes = BIT_RANGE(bgCnt, 7, 7);
		colorDepth = colorsPalettes ? 8u : 4u;
		mapDataBase = VRAM_START + BIT_RANGE(bgCnt, 8, 12) * BG_MAP_DATA_UNITSIZE;
		wrapAround = BIT_RANGE(bgCnt, 13, 13) || ID == 0 || ID == 1;
		screenSize = BIT_RANGE(bgCnt, 14, 15);
	}
	 U8 ID;
 	 U8 priority;
	 U32 tileDataBase;
	 bool mosaic;
	 uint_fast8_t colorsPalettes;
	 uint_fast8_t colorDepth;

 	 U32 mapDataBase;
	 bool wrapAround;
	 uint_fast8_t screenSize;
};