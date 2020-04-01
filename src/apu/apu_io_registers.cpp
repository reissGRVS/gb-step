#include "apu/apu.hpp"
#include "memory/regions.hpp"
#include "utils.hpp"

U32 APU::Read(const AccessSize& size, U32 address, const Sequentiality&)
{
	U32 actualIndex = address - APU_IO_START;
	auto value = ReadToSize(registers, actualIndex, size);

	return value;
}

void APU::Write(const AccessSize& size, U32 address, U32 value, const Sequentiality&)
{

	if (address == FIFO_A || address == FIFO_A + 2) {
		fifo[0].Push((S8)BIT_RANGE(value, 0, 7));
		fifo[0].Push((S8)BIT_RANGE(value, 8, 15));
	}

	if (address == FIFO_B || address == FIFO_B + 2) {
		fifo[1].Push((S8)BIT_RANGE(value, 0, 7));
		fifo[1].Push((S8)BIT_RANGE(value, 8, 15));
	}
	U32 actualIndex = address - APU_IO_START;
	WriteToSize(registers, actualIndex, value, size);
}
