#pragma once

#include "memory/cart_backup.hpp"
#include "memory/regions.hpp"
#include <array>
#include "int.hpp"

enum FlashSize { Single = 0,
	Double = 1 };
class Flash : public CartBackup {
public:
	Flash(FlashSize size)
		: size(size)
		, manufacturerID((size == Single) ? 0xBF : 0xC2)
		, deviceID((size == Single) ? 0xD4 : 0x09)
	{
	}

	U8 Read(U32 address) override;
	void Write(U32 address, U8 value) override;

private:
	void HandleOperation(U8 value);
	void EraseSector(U32 sector);

	// TODO: Support Atmel devices?
	// TODO: Terminate command for Macronix devices

	enum State { INIT0,
		INIT1,
		OPERATION,
		ACCEPT_WRITE,
		ACCEPT_BANK_SWITCH };
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

	std::array<U8, FLASH_BANK_SIZE> bank1{ 0xFF };
	// Only used for FLASH1M
	std::array<U8, FLASH_BANK_SIZE> bank2{ 0xFF };
};