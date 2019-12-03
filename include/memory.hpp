#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>

#include "joypad.hpp"
#include "memory_regions.hpp"

class Memory {
 public:
  Memory(std::string biosPath, std::string romPath, Joypad& joypad);

  enum Sequentiality { NSEQ, SEQ, PPU, DEBUG };

  enum AccessSize { Byte = 0xFFu, Half = 0xFFFFu, Word = 0xFFFFFFFFu };

  std::uint32_t Read(AccessSize size,
                     std::uint32_t address,
                     Sequentiality type);
  void Write(AccessSize size,
             std::uint32_t address,
             std::uint32_t value,
             Sequentiality type);

  std::uint16_t getHalf(const std::uint32_t& address) {
	return Read(Memory::AccessSize::Half, address, Memory::Sequentiality::PPU);
  }
  std::uint32_t getWord(const std::uint32_t& address) {
	return Read(Memory::AccessSize::Word, address, Memory::Sequentiality::PPU);
  }

  void setHalf(const std::uint32_t& address, const std::uint16_t& value) {
	Write(Memory::AccessSize::Half, address, value, Memory::Sequentiality::PPU);
  }
  void setWord(const std::uint32_t& address, const std::uint32_t& value) {
	Write(Memory::AccessSize::Word, address, value, Memory::Sequentiality::PPU);
  }

  void SetIOWriteCallback(std::uint32_t address,
                          std::function<void()> callback);
  void SetDebugWriteCallback(std::function<void(std::uint32_t)> callback);

 private:
  std::unordered_map<std::uint32_t, std::function<void()>> ioCallbacks;
  std::uint32_t ReadToSize(std::uint8_t* byte, AccessSize size);
  void WriteToSize(std::uint8_t* byte, std::uint32_t value, AccessSize size);

  // https://problemkaputt.de/gbatek.htm#gbamemorymap
  std::function<void(std::uint32_t)> PublishWriteCallback;
  Joypad& joypad;
  struct MemoryMap {
	struct {
	  std::array<std::uint8_t, BIOS_SIZE> bios{};
	  std::array<std::uint8_t, WRAMB_SIZE> wramb{};
	  std::array<std::uint8_t, WRAMC_SIZE> wramc{};
	  std::array<std::uint8_t, IOREG_SIZE> ioreg{};
	} gen;

	struct {
	  std::array<std::uint8_t, PRAM_SIZE> pram{};
	  std::array<std::uint8_t, VRAM_SIZE> vram{};
	  std::array<std::uint8_t, OAM_SIZE> oam{};
	} disp;

	struct {
	  // TODO: Make this as small as possible when rom is loaded?
	  std::array<std::uint8_t, ROM_SIZE> rom{};
	} ext;
  } mem;
};
