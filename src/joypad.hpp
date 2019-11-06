#pragma once
#include <cstdint>

class Joypad {
 public:
  void keyUpdate();
  std::uint16_t getKeyStatus();

 private:
  // Key state
  bool a;
  bool b;
  bool select;
  bool start;

  bool right;
  bool left;
  bool up;
  bool down;

  bool rightBump;
  bool leftBump;
};