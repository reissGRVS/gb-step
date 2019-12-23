#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "joypad.hpp"
#include "memory/cart_backup.hpp"
#include "memory/regions.hpp"
#include "system_clock.hpp"
#include "types.hpp"
#include "utils.hpp"

class Memory {
 public:
  Memory(std::shared_ptr<SystemClock> clock,
         std::string biosPath,
         std::string romPath,
         Joypad& joypad);

  std::uint32_t Read(AccessSize size,
                     std::uint32_t address,
                     Sequentiality type);
  void Write(AccessSize size,
             std::uint32_t address,
             std::uint32_t value,
             Sequentiality type);

  std::uint8_t GetByte(const std::uint32_t& address) {
	return Read(AccessSize::Byte, address, Sequentiality::FREE);
  }
  std::uint16_t GetHalf(const std::uint32_t& address) {
	return Read(AccessSize::Half, address, Sequentiality::FREE);
  }
  std::uint32_t GetWord(const std::uint32_t& address) {
	return Read(AccessSize::Word, address, Sequentiality::FREE);
  }
  void SetByte(const std::uint32_t& address, const std::uint8_t& value) {
	Write(AccessSize::Byte, address, value, Sequentiality::FREE);
  }
  void SetHalf(const std::uint32_t& address, const std::uint16_t& value) {
	Write(AccessSize::Half, address, value, Sequentiality::FREE);
  }
  void SetWord(const std::uint32_t& address, const std::uint32_t& value) {
	Write(AccessSize::Word, address, value, Sequentiality::FREE);
  }

  void RequestInterrupt(Interrupt i) {
	auto intReq = GetHalf(IF);
	BIT_SET(intReq, i);
	SetHalf(IF, intReq);
  }

  void SetIOWriteCallback(std::uint32_t address,
                          std::function<void(std::uint32_t)> callback);
  void SetDebugWriteCallback(std::function<void(std::uint32_t)> callback);

 private:
  std::shared_ptr<SystemClock> clock;
  std::unordered_map<std::uint32_t, std::function<void(std::uint32_t)>>
      ioCallbacks;

  std::string FindBackupID(size_t length);

  std::uint32_t ReadToSize(std::uint8_t* byte, AccessSize size);
  void WriteToSize(std::uint8_t* byte, std::uint32_t value, AccessSize size);

  void Tick(AccessSize size, std::uint32_t page, Sequentiality seq);
  void TickBySize(AccessSize size,
                  std::uint32_t ticks8,
                  std::uint32_t ticks16,
                  std::uint32_t ticks32);
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
	  std::array<std::uint8_t, ROM_SIZE> rom{};
	  std::unique_ptr<CartBackup> backup;
	} ext;
  } mem;
};
