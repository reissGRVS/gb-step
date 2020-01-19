#pragma once

#include "int.hpp"
#include <array>
#include <string>

const std::string EEPROM_V = "EEPROM_V";
const std::string SRAM_V = "SRAM_V";
const std::string FLASH_V = "FLASH_V";
const std::string FLASH512_V = "FLASH512_V";
const std::string FLASH1M_V = "FLASH1M_V";
const std::array<std::string, 5> BACKUP_ID_STRINGS = { EEPROM_V, SRAM_V, FLASH_V,
	FLASH512_V, FLASH1M_V };

class CartBackup {
public:
	virtual U8 Read(U32 address) = 0;
	virtual void Write(U32 address, U8 value) = 0;
	virtual ~CartBackup() = default;
};
