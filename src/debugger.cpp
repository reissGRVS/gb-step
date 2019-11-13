#include "debugger.hpp"
#include <iomanip>
#include <optional>
#include <string>

Debugger::debugOp Debugger::stringToOp(std::string str) {
  if (str == "step" || str == "s")
	return debugOp::step;
  if (str == "run" || str == "r")
	return debugOp::run;
  if (str == "log" || str == "l")
	return debugOp::toggleLog;
  if (str == "bp")
	return debugOp::addBPCondition;
  if (str == "registers" || str == "pr")
	return debugOp::printRegisters;
  if (str == "membps" || str == "pm")
	return debugOp::printMemBPs;
  if (str == "cbp")
	return debugOp::clearBPConditions;

  return debugOp::noOp;
}

std::vector<std::string> splitString(std::string str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream inputStream(str);

  for (std::string token; std::getline(inputStream, token, delimiter);) {
	tokens.push_back(token);
  }
  return tokens;
}

std::optional<std::uint32_t> getIntSafe(std::string str, int base = 10) {
  try {
	std::uint32_t stepCount = std::stoi(str, 0, base);
	return stepCount;
  } catch (std::exception& e) {
	return {};
  }
}

std::vector<std::string> Debugger::getUserInputTokens() {
  // Get input
  std::cout << ">> ";
  std::string input;
  std::getline(std::cin, input);

  return splitString(input, ' ');
}

void Debugger::onBreakpoint() {
  while (true) {
	auto tokens = getUserInputTokens();
	// Manage input
	if (tokens.size() > 0) {
	  auto op = stringToOp(tokens[0]);

	  switch (op) {
		case debugOp::step: {
		  if (tokens.size() == 2) {
			auto stepSize = getIntSafe(tokens[1]);
			if (stepSize.has_value())
			  setStep(stepSize.value());
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
		case debugOp::addBPCondition: {
		  if (tokens.size() >= 2) {
			addBreakpointCondition(tokens[1]);
		  }
		  break;
		}
		case debugOp::clearBPConditions: {
		  clearBreakpoints();
		  break;
		}
		case debugOp::printRegisters: {
		  printRegView();
		  break;
		}
		case debugOp::printMemBPs: {
		  printMemoryBPs();
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

void Debugger::printMemoryBPs() {
  std::cout << "Watching:" << std::endl;
  for (const auto& addr : watchedAddresses) {
	std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << addr
	          << std::endl;
  }
}

void Debugger::printRegView() {
  for (unsigned int i = 0; i < 16; i++) {
	std::cout << "R" << i << "=" << std::setfill('0') << std::setw(8)
	          << std::hex << view[i] << " ";
	if (i % 4 == 3) {
	  std::cout << std::endl;
	}
  }
  std::cout << std::endl << "BREAKPOINT REGISTERS" << std::endl;
  for (unsigned int i = 0; i < 16; i++) {
	auto reg = regWatchedValues[i];
	if (reg.has_value()) {
	  std::cout << "R" << i << "=" << std::setfill('0') << std::setw(8)
	            << std::hex << reg.value() << " ";
	}
  }
  std::cout << std::endl;
}

void Debugger::checkForBreakpoint(ARM7TDMI::RegisterView view_) {
  view = view_;
  // STEP BASED BREAKPOINTS
  if (steps_till_next_break == 0) {
	steps_till_next_break--;
	onBreakpoint();
	return;
  } else if (steps_till_next_break > 0) {
	steps_till_next_break--;
  }

  // REGISTER BASED BREAKPOINTS
  if (!noRegBP) {
	bool regMatch = true;
	for (unsigned int i = 0; i < 16; i++) {
	  if (regWatchedValues[i].has_value() &&
	      view[i] != regWatchedValues[i].value()) {
		regMatch = false;
		break;
	  }
	}
	if (regMatch) {
	  std::cout << "REGISTER BREAKPOINT" << std::endl;
	  onBreakpoint();
	  return;
	}
  }

  if (memoryBreakpoint) {
	std::cout << "MEMORY BREAKPOINT" << std::endl;
	onBreakpoint();
	exit(-1);
	return;
  }
}

void Debugger::clearBreakpoints() {
  regWatchedValues.fill(std::optional<std::uint32_t>());
  noRegBP = true;
  watchedAddresses.clear();
}

void Debugger::notifyMemoryWrite(std::uint32_t address) {
  memoryBreakpoint =
      !(watchedAddresses.find(address) == watchedAddresses.end());
}

void Debugger::setRun() {
  steps_till_next_break = -1;
}

void Debugger::setStep(int x) {
  steps_till_next_break = x;
}

void Debugger::addBreakpointCondition(std::string condition) {
  if (condition.empty())
	return;

  auto tokens = splitString(condition, '=');
  if (tokens.size() > 1 && condition.at(0) == 'R' && tokens[0].size() > 1) {
	auto reg = getIntSafe(tokens[0].substr(1));
	auto val = getIntSafe(tokens[1]);
	if (reg.has_value() && val.has_value()) {
	  noRegBP = false;
	  regWatchedValues[reg.value()] = val.value();
	}
  } else if (condition.at(0) == 'M') {
	if (tokens[0].size() > 1) {
	  auto addr = getIntSafe(tokens[0].substr(1), 16);
	  if (addr.has_value()) {
		watchedAddresses.insert(addr.value());
	  }
	}
  }
}

void Debugger::toggleLoggingLevel() {
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