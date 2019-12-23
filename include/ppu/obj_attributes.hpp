#pragma once

#include "memory/regions.hpp"
#include "utils.hpp"
#include "int.hpp"

struct ObjAttributes {
	ObjAttributes(const uint16_t& objAttr0,
		const uint16_t& objAttr1,
		const uint16_t& objAttr2)
	{
		attr0.full = objAttr0;
		attr1.full = objAttr1;
		attr2.full = objAttr2;
	}

	union {
		uint16_t full;
		struct {
			uint8_t yCoord : 8;
			uint8_t rotationScalingFlag : 1;
			uint8_t objDisable : 1;
			uint8_t objMode : 2;
			uint8_t objMosaic : 1;
			uint8_t colorsPalettes : 1;
			uint8_t objShape : 2;
		} b;
	} attr0;

	union {
		uint16_t full;
		struct {
			uint16_t xCoord : 9;
			uint8_t unused : 3;
			uint8_t horizontalFlip : 1;
			uint8_t verticalFlip : 1;
			uint8_t objSize : 2;
		} b;
	} attr1;

	union {
		uint16_t full;
		struct {
			uint16_t characterName : 10;
			uint8_t priority : 2;
			uint8_t paletteNumber : 4;
		} b;
	} attr2;
};
