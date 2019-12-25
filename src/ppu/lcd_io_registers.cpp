#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"

U32 PPU::Read(AccessSize size,
	U32 address,
	Sequentiality)
{
	U32 actualIndex = address - LCD_IO_START;
	auto value = ReadToSize(registers, actualIndex, size);
	spdlog::get("std")->trace("PPU Read {:X} @ {:X}/{:X}", value, address, actualIndex);
	return value;
}

void PPU::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality)
{
	U32 actualIndex = address - LCD_IO_START;

	spdlog::get("std")->trace("PPU Write {:X} @ {:X}/{:X}", value, address, actualIndex);
	WriteToSize(registers, actualIndex, value, size);
}