#pragma once

#include <spdlog/spdlog.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

class Debugger {
 public:
  int steps_till_next_break = 1;

  enum debugOp { step, run, toggleLog, noOp };

  debugOp stringToOp(std::string str) {
	if (str == "step" || str == "s")
	  return debugOp::step;
	if (str == "run" || str == "r")
	  return debugOp::run;
	if (str == "log" || str == "l")
	  return debugOp::toggleLog;

	return debugOp::noOp;
  };

  std::vector<std::string> getUserInputTokens() {
	// Get input
	std::cout << ">> ";
	std::string input;
	std::getline(std::cin, input);

	std::vector<std::string> tokens;
	std::stringstream inputStream(input);

	for (std::string token; std::getline(inputStream, token, ' ');) {
	  tokens.push_back(token);
	}
	return tokens;
  }
  void onBreakpoint() {
	while (true) {
	  auto tokens = getUserInputTokens();
	  // Manage input
	  if (tokens.size() > 0) {
		auto op = stringToOp(tokens[0]);

		switch (op) {
		  case debugOp::step: {
			try {
			  if (tokens.size() >= 2) {
				auto stepCount = std::stoi(tokens[1]);
				setStep(stepCount);
			  } else {
				std::cout << "No step count provided";
			  }
			} catch (std::exception& e) {
			  std::cout << e.what() << '\n';
			}
			return;
		  }
		  case debugOp::run: {
			setRun();
			return;
		  }
		  case debugOp::toggleLog: {
			toggleLoggingLevel();
			break;
		  }
		  case debugOp::noOp: {
			std::cout << "unsupported debugger op" << std::endl;
			break;
		  }
		}
	  }
	}
  }

  void checkForBreakpoint() {
	if (steps_till_next_break == 0) {
	  steps_till_next_break--;
	  onBreakpoint();
	  return;
	} else if (steps_till_next_break < 0) {
	  return;
	} else {
	  steps_till_next_break--;
	}
  }

  void setRun() { steps_till_next_break = -1; }

  void setStep(int x) {
	std::cout << "Set steps to " << x << std::endl;
	steps_till_next_break = x;
  }

  bool debugLogging = false;
  void toggleLoggingLevel() {
	if (debugLogging) {
	  std::cout << "Toggled to INFO log level" << std::endl;
	  spdlog::set_level(spdlog::level::info);
	  debugLogging = false;
	} else {
	  std::cout << "Toggled to DEBUG log level" << std::endl;
	  spdlog::set_level(spdlog::level::debug);
	  debugLogging = true;
	}
  }
};
