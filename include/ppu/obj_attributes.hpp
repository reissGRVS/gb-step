#pragma once

#include <cstdint>
#include "memory_regions.hpp"
#include "utils.hpp"

struct ObjAttributes {
  ObjAttributes(const uint16_t& objAttr0,
                const uint16_t& objAttr1,
                const uint16_t& objAttr2) {
	attr0.full = objAttr0;
	attr1.full = objAttr1;
	attr2.full = objAttr2;
  }

  union {
	uint16_t full;
	struct {
	  unsigned yCoord : 8;
	  unsigned rotationScalingFlag : 1;
	  unsigned objDisable : 1;
	  unsigned objMode : 2;
	  unsigned objMosaic : 1;
	  unsigned colorsPalettes : 1;
	  unsigned objShape : 2;
	} b;
  } attr0;

  union {
	uint16_t full;
	struct {
	  unsigned xCoord : 9;
	  unsigned unused : 3;
	  unsigned horizontalFlip : 1;
	  unsigned verticalFlip : 1;
	  unsigned objSize : 2;
	} b;
  } attr1;

  union {
	uint16_t full;
	struct {
	  unsigned characterName : 10;
	  unsigned priority : 2;
	  unsigned paletteNumber : 4;
	} b;
  } attr2;
};
