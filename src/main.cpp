
#include "debugger.hpp"
#include "gba.hpp"
#include "platform/sfml/joypad.hpp"
#include "platform/sfml/window.hpp"
#include <iostream>
#include <string>
#include <utility>

int main(int argc, char* argv[])
{

	if (argc != 3) {
		std::cerr << "Wrong number of args" << std::endl;
		return -1;
	}

	WindowSFML window;
	std::string biosPath = argv[1];
	std::string romPath = argv[2];
	GBAConfig cfg { biosPath, romPath, window, window.joypad };
	GBA gba(std::move(cfg));
	gba.run();
}