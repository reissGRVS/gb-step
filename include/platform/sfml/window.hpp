#pragma once

#include "../../joypad.hpp"
#include "../../screen.hpp"
#include <SFML/Graphics.hpp>

class WindowSFML : public Screen {
public:
	WindowSFML();
	void render(const Framebuffer& fb) override;

	Joypad joypad;

private:
	void translateFramebuffer(const Framebuffer& fb);

	std::array<sf::Uint8, 4 * SCREEN_TOTAL> sfFramebuffer;
	sf::Sprite b;
	sf::RenderWindow window;
	sf::Texture background;
};