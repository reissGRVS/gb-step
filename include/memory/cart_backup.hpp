#pragma once

#include <cstdint>

const std::string EEPROM_V = "EEPROM_V";
const std::string SRAM_V = "SRAM_V";
const std::string FLASH_V = "FLASH_V";
const std::string FLASH512_V = "FLASH512_V";
const std::string FLASH1M_V = "FLASH1M_V";
const std::array<std::string, 5> BACKUP_ID_STRINGS = {EEPROM_V, SRAM_V, FLASH_V,
                                                      FLASH512_V, FLASH1M_V};

class CartBackup {
 public:
  virtual std::uint8_t Read(std::uint32_t address) = 0;
  virtual void Write(std::uint32_t address, std::uint8_t value) = 0;
};
