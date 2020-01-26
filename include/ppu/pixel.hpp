
#include "utils.hpp"
#include "int.hpp"

struct Pixel {
	Pixel(U16 value) :
	r(BIT_RANGE(value, 0, 4)),
	g(BIT_RANGE(value, 5, 9)),
	b(BIT_RANGE(value, 10, 14)) {}

	void Blend(const Pixel& other, U8 eva, U8 evb)
	{
		r = (r * eva + other.r * evb)/16;
		if (r > 31) {r = 31;}
		g = (g * eva + other.g * evb)/16;
		if (g > 31) {g = 31;}
		b = (b * eva + other.b * evb)/16;
		if (b > 31) {b = 31;}
	}

	void BrightnessIncrease(U8 evy)
	{
		r = r + ((31-r)*evy)/16;
		g = g + ((31-g)*evy)/16;
		b = b + ((31-b)*evy)/16;
	}
	
	void BrightnessDecrease(U8 evy)
	{
		r = r - (r*evy/16);
		g = g - (g*evy/16);
		b = b - (b*evy/16);
	}

	U16 Value()
	{
		U16 v = r + (g << 5) + + (b << 10);
		return v;
	}

	U16 r;
	U16 g;
	U16 b;
};