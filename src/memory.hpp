#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include "joypad.hpp"
#include "memory_regions.hpp"

class Memory {
 public:
  Memory(std::string biosPath, std::string romPath, Joypad& joypad);

  enum Sequentiality { NSEQ, SEQ, PPU };

  enum AccessSize { Byte = 0xFF, Half = 0xFFFF, Word = 0xFFFFFFFF };

  std::uint32_t Read(AccessSize size,
                     std::uint32_t address,
                     Sequentiality type);
  void Write(AccessSize size,
             std::uint32_t address,
             std::uint32_t value,
             Sequentiality type);

 private:
  std::uint32_t ReadToSize(std::uint8_t* byte, AccessSize size);
  void WriteToSize(std::uint8_t* byte, std::uint32_t value, AccessSize size);

  // https://problemkaputt.de/gbatek.htm#gbamemorymap

  Joypad& joypad;
  struct MemoryMap {
	struct {
	  std::array<std::uint8_t, bios_size> bios{};
	  std::array<std::uint8_t, wramb_size> wramb{};
	  std::array<std::uint8_t, wramc_size> wramc{};
	  std::array<std::uint8_t, ioreg_size> ioreg{};
	} gen;

	struct {
	  std::array<std::uint8_t, pram_size> pram{};
	  std::array<std::uint8_t, vram_size> vram{};
	  std::array<std::uint8_t, oam_size> oam{};
	} disp;

	struct {
	  // TODO: Make this as small as possible when rom is loaded?
	  std::array<std::uint8_t, rom_size> rom{};
	} ext;
  } mem;
};
