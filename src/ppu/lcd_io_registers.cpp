#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"

U32 PPU::Read(const AccessSize& size, U32 address, const Sequentiality&) {
  U32 actualIndex = address - LCD_IO_START;
  auto value = ReadToSize(registers, actualIndex, size);

  return value;
}

void PPU::Write(const AccessSize& size, U32 address, U32 value, const Sequentiality&) {
  U32 actualIndex = address - LCD_IO_START;

  WriteToSize(registers, actualIndex, value, size);
}