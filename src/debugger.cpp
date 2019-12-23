#include "debugger.hpp"
#include "joypad.hpp"
#include <iomanip>
#include <optional>
#include <string>

Debugger::debugOp Debugger::StringToOp(std::string str)
{
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
	if (str == "memory" || str == "pm")
		return debugOp::printMemSection;
	if (str == "membps")
		return debugOp::printMemBPs;
	if (str == "w" || str == "write")
		return debugOp::writeHalfToMem;
	if (str == "cbp")
		return debugOp::clearBPConditions;
	if (str == "bt" || str == "backtrace")
		return debugOp::backtrace;

	return debugOp::noOp;
}

std::vector<std::string> SplitString(std::string str, char delimiter)
{
	std::vector<std::string> tokens;
	std::stringstream inputStream(str);

	for (std::string token; std::getline(inputStream, token, delimiter);) {
		tokens.push_back(token);
	}
	return tokens;
}

std::optional<std::uint32_t> GetIntSafe(std::string str, int base = 10)
{
	try {
		std::uint32_t stepCount = std::stoi(str, 0, base);
		return stepCount;
	} catch (std::exception& e) {
		return {};
	}
}

std::vector<std::string> Debugger::GetUserInputTokens()
{
	// Get input
	std::cout << ">> ";
	std::string input;
	std::getline(std::cin, input);

	return SplitString(input, ' ');
}

void Debugger::OnBreakpoint()
{
	while (true) {
		auto tokens = GetUserInputTokens();
		// Manage input
		if (tokens.size() > 0) {
			auto op = StringToOp(tokens[0]);

			switch (op) {
			case debugOp::step: {
				if (tokens.size() == 2) {
					auto stepSize = GetIntSafe(tokens[1]);
					if (stepSize.has_value())
						SetStep(stepSize.value());
				}
				return;
			}
			case debugOp::run: {
				SetRun();
				return;
			}
			case debugOp::toggleLog: {
				ToggleLoggingLevel();
				break;
			}
			case debugOp::addBPCondition: {
				if (tokens.size() >= 2) {
					AddBreakpointCondition(tokens[1]);
				}
				break;
			}
			case debugOp::clearBPConditions: {
				ClearBreakpoints();
				break;
			}
			case debugOp::printRegisters: {
				PrintRegView();
				break;
			}
			case debugOp::printMemBPs: {
				PrintMemoryBPs();
				break;
			}
			case debugOp::printMemSection: {
				if (tokens.size() > 1) {
					auto address = GetIntSafe(tokens[1], 16);
					if (address.has_value()) {
						if (tokens.size() == 2) {
							PrintMemorySection(address.value(), 4);
						} else if (tokens.size() == 3) {
							auto count = GetIntSafe(tokens[2], 16);
							if (count.has_value()) {
								PrintMemorySection(address.value(), count.value());
							}
						}
					}
				}

				break;
			}
			case debugOp::writeHalfToMem: {
				if (tokens.size() > 1) {
					auto address = GetIntSafe(tokens[1], 16);
					if (address.has_value()) {
						if (tokens.size() == 2) {
							WriteHalfToMemory(address.value(), 4);
						} else if (tokens.size() == 3) {
							auto count = GetIntSafe(tokens[2], 16);
							if (count.has_value()) {
								WriteHalfToMemory(address.value(), count.value());
							}
						}
					}
				}

				break;
			}
			case debugOp::backtrace: {
				view.backtrace.printBacktrace();
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

void Debugger::PrintMemoryBPs()
{
	std::cout << "Watching:" << std::endl;
	for (const auto& addr : watchedAddresses) {
		std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << addr
				  << std::endl;
	}
	std::cout << std::endl;
}

void Debugger::WriteHalfToMemory(std::uint32_t address, std::uint16_t value)
{
	memory->Write(Half, address, value, FREE);
}

void Debugger::PrintMemorySection(std::uint32_t address, std::uint32_t count)
{
	for (auto i = 0u; i < count; i++) {
		if (i % 4 == 0) {
			std::cout << std::endl
					  << "@0x" << std::setfill('0') << std::setw(8) << std::hex
					  << address + i << "  ";
		}
		std::cout << std::setfill('0') << std::setw(2) << std::hex
				  << memory->Read(Byte, address + i, FREE) << " ";
	}
	std::cout << std::endl;
}

void Debugger::PrintRegView()
{
	for (unsigned int i = 0; i < 16; i++) {
		std::cout << "R" << i << "=" << std::setfill('0') << std::setw(8)
				  << std::hex << view.registers[i] << " ";
		if (i % 4 == 3) {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl
			  << "BREAKPOINT REGISTERS" << std::endl;
	for (unsigned int i = 0; i < 16; i++) {
		auto reg = regWatchedValues[i];
		if (reg.has_value()) {
			std::cout << "R" << i << "=" << std::setfill('0') << std::setw(8)
					  << std::hex << reg.value() << " ";
		}
	}
	std::cout << std::endl;
}

void Debugger::CheckForBreakpoint(ARM7TDMI::StateView view_)
{
	view = view_;
	// STEP BASED BREAKPOINTS
	if (steps_till_next_break == 0) {
		steps_till_next_break--;
		OnBreakpoint();
		return;
	} else if (steps_till_next_break > 0) {
		steps_till_next_break--;
	}

	if (Joypad::getKey(Joypad::KEY::BP)) {
		OnBreakpoint();
		return;
	}

	// REGISTER BASED BREAKPOINTS
	if (!noRegBP) {
		bool regMatch = true;
		for (unsigned int i = 0; i < 16; i++) {
			if (regWatchedValues[i].has_value() && view.registers[i] != regWatchedValues[i].value()) {
				regMatch = false;
				break;
			}
		}
		if (regMatch) {
			std::cout << "REGISTER BREAKPOINT" << std::endl;
			OnBreakpoint();
			return;
		}
	}

	if (memoryBreakpoint) {
		std::cout << "MEMORY BREAKPOINT" << std::endl;
		OnBreakpoint();
		memoryBreakpoint = false;
		return;
	}
}

void Debugger::ClearBreakpoints()
{
	regWatchedValues.fill(std::optional<std::uint32_t>());
	noRegBP = true;
	watchedAddresses.clear();
}

void Debugger::NotifyMemoryWrite(std::uint32_t address)
{
	memoryBreakpoint = !(watchedAddresses.find(address) == watchedAddresses.end());
}

void Debugger::SetRun()
{
	steps_till_next_break = -1;
}

void Debugger::SetStep(int x)
{
	steps_till_next_break = x;
}

void Debugger::AddBreakpointCondition(std::string condition)
{
	if (condition.empty())
		return;

	auto tokens = SplitString(condition, '=');
	if (tokens.size() > 1 && condition.at(0) == 'R' && tokens[0].size() > 1) {
		auto reg = GetIntSafe(tokens[0].substr(1));
		auto val = GetIntSafe(tokens[1], 16);
		if (reg.has_value() && val.has_value()) {
			noRegBP = false;
			regWatchedValues[reg.value()] = val.value();
		}
	} else if (condition.at(0) == 'M') {
		if (tokens[0].size() > 1) {
			auto addr = GetIntSafe(tokens[0].substr(1), 16);
			if (addr.has_value()) {
				watchedAddresses.insert(addr.value());
			}
		}
	}
}

void Debugger::ToggleLoggingLevel()
{
	if (logLevel == spdlog::level::level_enum::trace) {
		std::cout << "Toggled to INFO log level" << std::endl;
		logLevel = spdlog::level::level_enum::info;
	} else if (logLevel == spdlog::level::level_enum::info) {
		std::cout << "Toggled to DEBUG log level" << std::endl;
		logLevel = spdlog::level::level_enum::debug;
	} else {
		std::cout << "Toggled to TRACE log level" << std::endl;
		logLevel = spdlog::level::level_enum::trace;
	}

	spdlog::set_level(logLevel);
}