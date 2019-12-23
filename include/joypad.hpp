#pragma once
#include "int.hpp"

class Joypad {
public:
	enum KEY {
		A,
		B,
		SELECT,
		START,
		RIGHT,
		LEFT,
		UP,
		DOWN,
		RIGHTBUMP,
		LEFTBUMP,
		BP
	};

	void keyUpdate();
	static bool getKey(KEY key)
	{
		switch (key) {
		case BP: {
			// BP key, should be turned off after getKey returns true as to stop the
			// debugger getting stuck
			if (bp) {
				bp = false;
				return true;
			} else {
				return false;
			}
		}

		default:
			return false;
		}
	}
	U16 getKeyStatus();

private:
	// Key state
	static bool a;
	static bool b;
	static bool select;
	static bool start;
	static bool right;
	static bool left;
	static bool up;
	static bool down;
	static bool rightBump;
	static bool leftBump;
	static bool bp;
};