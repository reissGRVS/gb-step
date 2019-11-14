#include "joypad.hpp"
#include <SFML/Window/Keyboard.hpp>
#include "utils.hpp"

void Joypad::keyUpdate() {
  right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
  left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
  up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
  down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
  a = sf::Keyboard::isKeyPressed(sf::Keyboard::K);
  b = sf::Keyboard::isKeyPressed(sf::Keyboard::L);
  rightBump = sf::Keyboard::isKeyPressed(sf::Keyboard::I);
  leftBump = sf::Keyboard::isKeyPressed(sf::Keyboard::O);
  select = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
  start = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);

  bp = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
}

std::uint16_t Joypad::getKeyStatus() {
  std::uint16_t keyStatus = NBIT_MASK(16);

  if (a)
	BIT_CLEAR(keyStatus, 0);
  if (b)
	BIT_CLEAR(keyStatus, 1);
  if (select)
	BIT_CLEAR(keyStatus, 2);
  if (start)
	BIT_CLEAR(keyStatus, 3);
  if (right)
	BIT_CLEAR(keyStatus, 4);
  if (left)
	BIT_CLEAR(keyStatus, 5);
  if (up)
	BIT_CLEAR(keyStatus, 6);
  if (down)
	BIT_CLEAR(keyStatus, 7);
  if (leftBump)
	BIT_CLEAR(keyStatus, 8);
  if (rightBump)
	BIT_CLEAR(keyStatus, 9);

  return keyStatus;
}

bool Joypad::a;
bool Joypad::b;
bool Joypad::select;
bool Joypad::start;
bool Joypad::right;
bool Joypad::left;
bool Joypad::up;
bool Joypad::down;
bool Joypad::rightBump;
bool Joypad::leftBump;
bool Joypad::bp;