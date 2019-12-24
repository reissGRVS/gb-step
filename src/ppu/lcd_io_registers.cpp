#include "ppu/ppu.hpp"

U32 PPU::Read(AccessSize size,
	U32 address,
	Sequentiality)
{
	U32 actualIndex = address - LCD_IO_START;
	return ReadToSize(&registers[actualIndex], size);
}

void PPU::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality)
{
	//TODO:special stuff, maybe on DISPCNT, BGNCNT
	U32 actualIndex = address - LCD_IO_START;
	WriteToSize(&registers[actualIndex], value, size);
}