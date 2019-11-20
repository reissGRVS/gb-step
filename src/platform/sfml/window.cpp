#include "platform/sfml/window.hpp"
#include "utils.hpp"

WindowSFML::WindowSFML()
    : window(sf::VideoMode(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE),
             "gb-step",
             sf::Style::Titlebar) {
  window.setFramerateLimit(60);
  background.create(SCREEN_WIDTH, SCREEN_HEIGHT);
  b.setTexture(background);
  b.scale(SCALE, SCALE);
}

void WindowSFML::render(const Framebuffer& fb) {
  translateFramebuffer(fb);
  joypad.keyUpdate();
  background.update(sfFramebuffer.data());
  window.clear();
  window.draw(b);

  sf::Event ev;
  window.pollEvent(ev);
  if (ev.key.code == sf::Keyboard::BackSpace &&
      ev.type == sf::Event::KeyPressed) {
	window.setFramerateLimit(1);
  }
  window.display();
}

void WindowSFML::translateFramebuffer(const Framebuffer& fb) {
  for (std::uint64_t i = 0; i < fb.size(); i++) {
	auto base = i * 4;
	const auto& pixel = fb[i];
	sfFramebuffer[base] = BIT_RANGE(pixel, 0, 4) << 3;
	sfFramebuffer[base + 1] = BIT_RANGE(pixel, 5, 9) << 3;
	sfFramebuffer[base + 2] = BIT_RANGE(pixel, 10, 14) << 3;
	sfFramebuffer[base + 3] = 255;
  }
}