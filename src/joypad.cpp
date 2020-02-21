#include "joypad.hpp"
#include "utils.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

void Joypad::keyUpdate() {
  right = sf::Keyboard::isKeyPressed(sf::Keyboard::D) || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)6) > 60);
  left = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)6) < -60);
  up = sf::Keyboard::isKeyPressed(sf::Keyboard::W)  || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)7) < -60);
  down = sf::Keyboard::isKeyPressed(sf::Keyboard::S)  || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)7) > 60);
  a = sf::Keyboard::isKeyPressed(sf::Keyboard::K) || sf::Joystick::isButtonPressed(0, 1);
  b = sf::Keyboard::isKeyPressed(sf::Keyboard::L) || sf::Joystick::isButtonPressed(0, 0);
  rightBump = sf::Keyboard::isKeyPressed(sf::Keyboard::I) || sf::Joystick::isButtonPressed(0, 5);
  leftBump = sf::Keyboard::isKeyPressed(sf::Keyboard::O) || sf::Joystick::isButtonPressed(0, 4);
  select = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) || sf::Joystick::isButtonPressed(0, 6);
  start = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) || sf::Joystick::isButtonPressed(0, 7);

  bp = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
  esc = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
}

U16 Joypad::getKeyStatus() {
  U16 keyStatus = NBIT_MASK(16);

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
bool Joypad::esc;