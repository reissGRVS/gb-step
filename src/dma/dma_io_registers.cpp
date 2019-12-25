#include "dma/controller.hpp"

namespace DMA {
U32 Controller::Read(AccessSize size,
	U32 address,
	Sequentiality)
{
	U32 actualIndex = address - DMA_IO_START;
	return ReadToSize(registers, actualIndex, size);
}

void Controller::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality)
{

	if (size == Byte) {
		spdlog::get("std")->error("Byte DMA Write - Needs implemented?");
		exit(-1);
	}

	switch (address) {
	case DMA0CNT_H: {
		CntHUpdateCallback(0, value);
		break;
	}
	case DMA1CNT_H: {
		CntHUpdateCallback(1, value);
		break;
	}
	case DMA2CNT_H: {
		CntHUpdateCallback(2, value);
		break;
	}
	case DMA3CNT_H: {
		CntHUpdateCallback(3, value);
		break;
	}
	default:
		break;
	}

	U32 actualIndex = address - DMA_IO_START;
	WriteToSize(registers, actualIndex, value, size);
}
}