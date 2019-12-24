#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"
#include <array>

class IRIORegisters : public ReadWriteInterface {

public:
	static const U32 IR_IO_START = 0x04000200, IR_IO_END = 0x04000210;
	static const U32 IR_IO_SIZE = IR_IO_END - IR_IO_START;

protected:
	std::array<U8, IR_IO_SIZE> ioregisters{};
};