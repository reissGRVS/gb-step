#pragma once

#include <array>
#include <cstdint>
#include "cart_backup.hpp"
#include "memory_regions.hpp"

enum FlashSize { Single = 0, Double = 1 };
class Flash : public CartBackup {
 public:
  Flash(FlashSize size)
      : size(size),
        manufacturerID((size == Single) ? 0xBF : 0xC2),
        deviceID((size == Single) ? 0xD4 : 0x09) {}

  std::uint8_t Read(std::uint32_t address) override;
  void Write(std::uint32_t address, std::uint8_t value) override;

 private:
  void HandleOperation(std::uint8_t value);
  void EraseSector(std::uint32_t sector);

  // TODO: Support Atmel devices?
  // TODO: Terminate command for Macronix devices

  enum State { INIT0, INIT1, OPERATION, ACCEPT_WRITE, ACCEPT_BANK_SWITCH };
  enum Operation {
	CHIP_ID_ENABLE = 0x90,
	CHIP_ID_DISABLE = 0xF0,
	ERASE = 0x80,
	ERASE_CHIP = 0x10,
	ERASE_SECTOR = 0x30,
	WRITE_SINGLE = 0xA0,
	BANK_SWITCH = 0xB0
  };

  FlashSize size;
  uint8_t manufacturerID;
  uint8_t deviceID;

  State state = INIT0;
  uint8_t currentBank = 0;
  bool idMode = false;
  bool eraseEnable = false;

  std::array<std::uint8_t, FLASH_BANK_SIZE> bank1{0xFF};
  // Only used for FLASH1M
  std::array<std::uint8_t, FLASH_BANK_SIZE> bank2{0xFF};
};