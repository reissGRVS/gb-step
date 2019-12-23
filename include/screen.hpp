#pragma once
#include <array>

class Screen {
public:
	static const std::uint32_t SCREEN_HEIGHT = 160, SCREEN_WIDTH = 240,
							   SCREEN_TOTAL = SCREEN_WIDTH * SCREEN_HEIGHT,
							   SCALE = 4;
	using Framebuffer = std::array<std::uint16_t, SCREEN_TOTAL>;

	virtual void render(const Framebuffer& fb) = 0;
};