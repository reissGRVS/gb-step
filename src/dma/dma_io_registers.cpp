#include "dma/controller.hpp"

namespace DMA {
U32 Controller::Read(AccessSize size,
	U32 address,
	Sequentiality)
{
	U32 actualIndex = address - DMA_IO_START;
	return ReadToSize(&registers[actualIndex], size);
}

void Controller::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality)
{
	//TODO:special stuff, for DMA
	U32 actualIndex = address - DMA_IO_START;
	WriteToSize(&registers[actualIndex], value, size);
}
}