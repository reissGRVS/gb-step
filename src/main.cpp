#include <iostream>
#include <string>

#include "gba.hpp"
#include "platform/sfml/window.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char* argv[]) {
  spdlog::set_level(spdlog::level::info);  // Set global log level to info
  // spdlog::set_pattern("[%H:%M:%S] [%^%L%$] %v");
  spdlog::set_pattern("%v");
  spdlog::info("Launching");

  if (argc != 3) {
	spdlog::error("Wrong number of args");
	return -1;
  }

  WindowSFML window;
  std::string biosPath = argv[1];
  std::string romPath = argv[2];
  GBAConfig cfg{biosPath, romPath, window, window.joypad};

  GBA gba(cfg);
  gba.run();
}