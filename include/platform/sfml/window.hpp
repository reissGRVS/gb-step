#pragma once

#include "../../screen.hpp"
#include "platform/sfml/joypad.hpp"
#include <SFML/Graphics.hpp>
#include <chrono>

class WindowSFML : public Screen {
public:
	WindowSFML();
	void render(const Framebuffer& fb) override;

	JoypadSFML joypad;

private:
	void translateFramebuffer(const Framebuffer& fb);

	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
	std::array<sf::Uint8, 4 * SCREEN_TOTAL> sfFramebuffer;
	sf::Sprite b;
	sf::RenderWindow window;
	sf::Texture background;
};