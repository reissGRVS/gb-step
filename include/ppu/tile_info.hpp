#pragma once

#include "int.hpp"

struct TileInfo {
	const U16 startX;
	const U16 startY;
	const U16 tileNumber;
	const U16 colorDepth;
	const U32 tileDataBase;
	const U8 verticalFlip;
	const U8 horizontalFlip;
	const U8 paletteNumber;
	const U8 priority;
};