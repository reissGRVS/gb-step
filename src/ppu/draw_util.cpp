#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "ppu/pixel.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

U16 PPU::GetBgColorFromSubPalette(const U8& paletteNumber,
	const U8& colorID,
	bool obj)
{
	return GetBgColorFromPalette(paletteNumber * 16u + colorID, obj);
}

U16 PPU::GetBgColorFromPalette(const U8& colorID,
	bool obj)
{
	auto paletteStart = PRAM_START;
	if (obj)
		paletteStart += 0x200;
	return memory->GetHalf(colorID * 2 + paletteStart);
}

void PPU::SetSFXPixel(OptPixel& firstPrioPixel, OptPixel& secondPrioPixel, U16& dest, BldCnt::ColorSpecialEffect effect)
{
	if (!firstPrioPixel.has_value())
		return;

	Pixel firstPixel{firstPrioPixel.value()};

	switch (effect)
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
