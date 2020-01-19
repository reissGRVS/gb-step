#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"
#include <array>

class DMAIORegisters : public ReadWriteInterface {

public:
	std::string Name() override { return "DMA_IO"; };
	static const U32 DMA_IO_START = 0x040000B0, DMA_IO_END = 0x040000E0;
	static const U32 DMA_IO_SIZE = DMA_IO_END - DMA_IO_START;

protected:
	std::array<U8, DMA_IO_SIZE> registers{};
};