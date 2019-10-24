#include <iostream>
#include <string>

#include "gba.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char* argv[]) {
  spdlog::set_level(spdlog::level::debug);  // Set global log level to info
  spdlog::set_pattern("[%H:%M:%S] [%^%L%$] %v");

  spdlog::info("Launching");

  if (argc != 3) {
	spdlog::error("Wrong number of args");
	return -1;
  }

  GBAConfig cfg;
  cfg.biosPath = argv[1];
  cfg.romPath = argv[2];

  GBA gba(cfg);
  gba.run();
}