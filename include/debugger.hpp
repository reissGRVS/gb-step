#pragma once

#include <spdlog/spdlog.h>
#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include "arm7tdmi/stateview.hpp"
#include "arm7tdmi/types.hpp"
#include "memory/memory.hpp"

class Debugger {
 public:
  Debugger(std::shared_ptr<Memory> memory) : memory(memory){};

  enum debugOp {
	step,
	run,
	toggleLog,
	addBPCondition,
	clearBPConditions,
	printRegisters,
	printMemBPs,
	printMemSection,
	writeHalfToMem,
	backtrace,
	noOp
  };

  void CheckForBreakpoint(ARM7TDMI::StateView view_);

  void NotifyMemoryWrite(std::uint32_t address);

 private:
  debugOp StringToOp(std::string str);
  std::vector<std::string> GetUserInputTokens();

  void OnBreakpoint();
  void PrintMemoryBPs();
  void PrintRegView();
  void PrintMemorySection(std::uint32_t address, std::uint32_t count);
  void WriteHalfToMemory(std::uint32_t address, std::uint16_t value);
  void ClearBreakpoints();
  void SetRun();
  void SetStep(int x);
  void AddBreakpointCondition(std::string condition);
  void ToggleLoggingLevel();

  std::shared_ptr<Memory> memory;
  int steps_till_next_break = 1;
  spdlog::level::level_enum logLevel = spdlog::level::level_enum::info;
  bool noRegBP = true;
  bool memoryBreakpoint = false;
  ARM7TDMI::StateView view;
  std::array<std::optional<std::uint32_t>, 16> regWatchedValues{};
  std::unordered_set<std::uint32_t> watchedAddresses;
};
