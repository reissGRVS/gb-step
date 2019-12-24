#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"

class TimersIORegisters : public ReadWriteInterface {

public:
	static const U32 TIMER_IO_START = 0x04000100, TIMER_IO_END = 0x04000112;
	static const U32 TIMER_IO_SIZE = TIMER_IO_END - TIMER_IO_START;

protected:
	std::array<U8, TIMER_IO_SIZE> registers{};
};