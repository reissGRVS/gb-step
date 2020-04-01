#include "joypad.hpp"
#include "utils.hpp"

U16 Joypad::getKeyStatus()
{
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