#pragma once

#include "int.hpp"
#include "utils.hpp"

struct BldCnt {
	BldCnt(){}

	BldCnt(U16 value)
	{
		for (U8 i = 0; i < firstTarget.size(); i++)
		{
			firstTarget[i] = BIT_RANGE(value, i, i);
			secondTarget[i] = BIT_RANGE(value, (i+8), (i+8));
		}
		colorSpecialEffect = (ColorSpecialEffect) BIT_RANGE(value, 6, 7);
	}

	enum TargetLayer {
		Bg0 = 0,
		Bg1 = 1,
		Bg2 = 2,
		Bg3 = 3,
		Sprites = 4,
		Backdrop = 5
	};
	std::array<bool, 6> firstTarget = {};
	
	enum ColorSpecialEffect {
		None = 0,
		AlphaBlending = 1,
		BrightnessIncrease = 2,
		BrightnessDecrease = 3
	};
	ColorSpecialEffect colorSpecialEffect = None;

	std::array<bool, 6> secondTarget = {};
};
