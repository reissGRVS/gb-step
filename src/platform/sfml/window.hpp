#pragma once

#include <SFML/Graphics.hpp>
#include "../../screen.hpp"

class WindowSFML : public Screen {
 public:
  WindowSFML();
  void render(const Framebuffer& fb) override;

 private:
  void translateFramebuffer(const Framebuffer& fb);
  std::array<sf::Uint8, 4 * SCREEN_TOTAL> sfFramebuffer;
  sf::Sprite b;
  sf::RenderWindow window;
  sf::Texture background;
};