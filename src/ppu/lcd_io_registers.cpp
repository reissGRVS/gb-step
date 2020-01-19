#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"

U32 PPU::Read(AccessSize size, U32 address, Sequentiality) {
  U32 actualIndex = address - LCD_IO_START;
  auto value = ReadToSize(registers, actualIndex, size);

  return value;
}

void PPU::Write(AccessSize size, U32 address, U32 value, Sequentiality) {
  U32 actualIndex = address - LCD_IO_START;

  WriteToSize(registers, actualIndex, value, size);
}