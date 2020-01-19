#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"

class TimersIORegisters : public ReadWriteInterface {

public:
	std::string Name() override { return "TIMERS_IO"; };
	static const U32 TIMER_IO_START = 0x04000100, TIMER_IO_END = 0x04000110;
};