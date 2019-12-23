#pragma once

#include "arm7tdmi/stateview.hpp"
#include "arm7tdmi/types.hpp"
#include "memory/memory.hpp"
#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <unordered_set>

class Debugger {
public:
	Debugger(std::shared_ptr<Memory> memory)
		: memory(memory){};

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

	void NotifyMemoryWrite(U32 address);

private:
	debugOp StringToOp(std::string str);
	std::vector<std::string> GetUserInputTokens();

	void OnBreakpoint();
	void PrintMemoryBPs();
	void PrintRegView();
	void PrintMemorySection(U32 address, U32 count);
	void WriteHalfToMemory(U32 address, U16 value);
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
	std::array<std::optional<U32>, 16> regWatchedValues{};
	std::unordered_set<U32> watchedAddresses;
};
