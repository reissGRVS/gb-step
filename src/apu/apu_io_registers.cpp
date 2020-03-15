#include "apu/apu.hpp"


U32 APU::Read(const AccessSize& size, U32 address, const Sequentiality&) {
  U32 actualIndex = address - APU_IO_START;
  auto value = ReadToSize(registers, actualIndex, size);

  return value;
}

void APU::Write(const AccessSize& size, U32 address, U32 value, const Sequentiality&) {
	U32 actualIndex = address - APU_IO_START;
	WriteToSize(registers, actualIndex, value, size);
}
