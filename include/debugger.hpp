#pragma once

#include <spdlog/spdlog.h>
#include <array>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include "arm7tdmi/types.hpp"

class Debugger {
 public:
  enum debugOp {
	step,
	run,
	toggleLog,
	addBPCondition,
	clearBPConditions,
	printRegisters,
	printMemBPs,
	noOp
  };

  void checkForBreakpoint(ARM7TDMI::RegisterView view_);

  void notifyMemoryWrite(std::uint32_t address);

 private:
  debugOp stringToOp(std::string str);
  std::vector<std::string> getUserInputTokens();

  void onBreakpoint();
  void printMemoryBPs();
  void printRegView();
  void clearBreakpoints();
  void setRun();
  void setStep(int x);
  void addBreakpointCondition(std::string condition);
  void toggleLoggingLevel();

  int steps_till_next_break = 1;
  bool debugLogging = false;
  bool noRegBP = true;
  bool memoryBreakpoint = false;
  ARM7TDMI::RegisterView view;
  std::array<std::optional<std::uint32_t>, 16> regWatchedValues{};
  std::unordered_set<std::uint32_t> watchedAddresses;
};
