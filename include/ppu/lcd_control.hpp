#pragma once

#include "int.hpp"
#include "utils.hpp"


struct DispCnt {
	DispCnt(U16 value) :
		bgMode{(U8)BIT_RANGE(value, 0, 2)},
		frameSelect{(U8)BIT_RANGE(value, 4, 4)},
		objMapping{(U8)BIT_RANGE(value, 6, 6)},
		forcedBlank{(bool)BIT_RANGE(value, 7, 7)},
		screenDisplay{(U8)BIT_RANGE(value, 8, 11)},
		objDisplay{(bool)BIT_RANGE(value, 12, 12)},
		win0Display{(bool)BIT_RANGE(value, 13, 13)},
		win1Display{(bool)BIT_RANGE(value, 14, 14)},
		objWindowDisplay{(bool)BIT_RANGE(value, 15, 15)}
	{}

	U8 bgMode;
	U8 frameSelect;
	U8 objMapping;
	bool forcedBlank;
	U8 screenDisplay;
	bool objDisplay;
	bool win0Display;
	bool win1Display;
	bool objWindowDisplay;
};
