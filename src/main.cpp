
#include <string>
#include <utility>

#include "debugger.hpp"
#include "gba.hpp"
#include "platform/sfml/window.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


int main(int argc, char* argv[]) {

  auto standardLog = spdlog::stdout_color_mt("std");
  spdlog::set_level(spdlog::level::info);  // Set global log level to info
  // spdlog::set_pattern("[%H:%M:%S] [%^%L%$] %v");
  spdlog::set_pattern("%v");
  spdlog::info("Launching");


  #ifndef NDEBUG
  spdlog::info("Debug mode");
  auto debugger = std::make_unique<Debugger>();
  #else
  std::unique_ptr<Debugger> debugger;
  #endif

  if (argc != 3) {
	spdlog::error("Wrong number of args");
	return -1;
  }

  WindowSFML window;
  std::string biosPath = argv[1];
  std::string romPath = argv[2];
  GBAConfig cfg{biosPath, romPath, window, window.joypad, std::move(debugger)};
  GBA gba(std::move(cfg));
  gba.run();
}