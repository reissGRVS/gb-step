#include "arm7tdmi/cpu.hpp"
namespace ARM7TDMI {
U32 CPU::Read(AccessSize size,
	U32 address,
	Sequentiality)
{
	U32 actualIndex = address - IR_IO_START;
	return ReadToSize(&ioregisters[actualIndex], size);
}

void CPU::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality)
{
	//TODO:special stuff, for Interrupt stuff
	U32 actualIndex = address - IR_IO_START;
	WriteToSize(&ioregisters[actualIndex], value, size);
}
}
