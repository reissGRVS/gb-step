#include "timers/timers.hpp"

U32 Timers::Read(AccessSize size,
	U32 address,
	Sequentiality)
{
	U32 actualIndex = address - TIMER_IO_START;
	return ReadToSize(&registers[actualIndex], size);
}

void Timers::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality)
{
	//TODO:special stuff, for DMA
	U32 actualIndex = address - TIMER_IO_START;
	WriteToSize(&registers[actualIndex], value, size);
}