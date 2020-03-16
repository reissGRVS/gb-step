#include "apu/apu.hpp"
#include "memory/regions.hpp"
#include "utils.hpp"

U32 APU::Read(const AccessSize& size, U32 address, const Sequentiality&) {
  U32 actualIndex = address - APU_IO_START;
  auto value = ReadToSize(registers, actualIndex, size);

  return value;
}

void APU::Write(const AccessSize& size, U32 address, U32 value, const Sequentiality&) {

	if (address == FIFO_A) 
	{
		fifo[0].Push((S8)BIT_RANGE(value, 0, 7));
		fifo[0].Push((S8)BIT_RANGE(value, 8, 15));
		fifo[0].Push((S8)BIT_RANGE(value, 16, 23));
		fifo[0].Push((S8)BIT_RANGE(value, 24, 31));
	}
	if (address == FIFO_B) 
	{
		fifo[1].Push((S8)BIT_RANGE(value, 0, 7));
		fifo[1].Push((S8)BIT_RANGE(value, 8, 15));
		fifo[1].Push((S8)BIT_RANGE(value, 16, 23));
		fifo[1].Push((S8)BIT_RANGE(value, 24, 31));
	}
	U32 actualIndex = address - APU_IO_START;
	WriteToSize(registers, actualIndex, value, size);
}
