#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"

class APUIORegisters : public ReadWriteInterface {

public:
	std::string Name() override { return "APU_IO"; };
	static const U32 APU_IO_START = 0x04000060, APU_IO_END = 0x040000A8;
	static const U32 APU_IO_SIZE = APU_IO_END - APU_IO_START;
protected:
	std::array<U8, APU_IO_SIZE> registers{};
};