#pragma once
#include <array>

class Screen {
public:
	static const U32 SCREEN_HEIGHT = 160, SCREEN_WIDTH = 240,
							   SCREEN_TOTAL = SCREEN_WIDTH * SCREEN_HEIGHT,
							   SCALE = 4;
	using Framebuffer = std::array<U16, SCREEN_TOTAL>;

	virtual void render(const Framebuffer& fb) = 0;
};