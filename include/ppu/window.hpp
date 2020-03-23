#pragma once

#include "utils.hpp"
#include "int.hpp"

enum WindowID {
	Win0,
	Win1,
	Outside,
	Obj,
};

struct Window {

	bool InRange(U16 x, U16 y)
	{
		 return (x >= X1 && x <= X2 && y >= Y1 && y <= Y2 );
	}

	void SetXValues(U16 value)
	{
		X2 = BIT_RANGE(value, 0, 7);
		X1 = BIT_RANGE(value, 8, 15);
	}
	U8 X1;
	U8 X2;

	void SetYValues(U16 value)
	{
		Y2 = BIT_RANGE(value, 0, 7);
		Y1 = BIT_RANGE(value, 8, 15);
	}
	U8 Y1;
	U8 Y2;


	void SetSettings(U8 value)
	{
		for (int i = 0; i < 4; i++) bgEnable[i] = BIT_RANGE(value, i, i);
		objEnable = BIT_RANGE(value, 4, 4);
		sfxEnable = BIT_RANGE(value, 5, 5);
	}
	bool bgEnable[4];
	bool objEnable;
	bool sfxEnable;
};
