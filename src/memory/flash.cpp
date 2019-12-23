#include "memory/flash.hpp"
#include "spdlog/spdlog.h"
#include <algorithm>

U8 Flash::Read(U32 address)
{
	address &= FLASH_BANK_MASK;

	if (idMode) {
		if (address == 0) {
			spdlog::get("std")->info("Returning manID");
			return manufacturerID;
		} else if (address == 1) {
			spdlog::get("std")->info("Returning devID");
			return deviceID;
		}
	}

	if (currentBank == 0) {
		return bank1[address];
	} else {
		return bank2[address];
	}
}

void Flash::Write(U32 address, U8 value)
{
	address &= FLASH_BANK_MASK;

	switch (state) {
	case INIT0: {
		if (value == 0xAA && address == 0x5555) {
			state = INIT1;
		}
		break;
	}
	case INIT1: {
		if (value == 0x55 && address == 0x2AAA) {
			state = OPERATION;
		} else {
			state = INIT0;
		}
		break;
	}
	case OPERATION: {
		if (address == 0x5555 && value != ERASE_SECTOR) {
			HandleOperation(value);
		} else if (value == ERASE_SECTOR && (address % 0x1000) == 0) {
			EraseSector(address / 0x1000);
		} else {
			state = INIT0;
		}
		break;
	}
	case ACCEPT_WRITE: {
		if (currentBank == 0) {
			bank1[address] = value;
		} else {
			bank2[address] = value;
		}
		state = INIT0;
		break;
	}
	case ACCEPT_BANK_SWITCH: {
		if (value < 2u) {
			currentBank = value;
		}
		state = INIT0;
		break;
	}
	}
}

void Flash::EraseSector(U32 sector)
{
	const auto SECTOR_SIZE = 0x1000u;
	auto bankStart = currentBank ? bank2.begin() : bank1.begin();
	auto sectorStart = bankStart + SECTOR_SIZE * sector;
	std::fill_n(sectorStart, SECTOR_SIZE, 0xFF);
	state = INIT0;
}

void Flash::HandleOperation(U8 value)
{
	switch (value) {
	case CHIP_ID_ENABLE: {
		spdlog::get("std")->info("CHIP ID ENABLE");
		idMode = true;
		state = INIT0;
		break;
	}
	case CHIP_ID_DISABLE: {
		spdlog::get("std")->info("CHIP ID DISABLE");
		idMode = false;
		state = INIT0;
		break;
	}
	case ERASE: {
		eraseEnable = true;
		state = INIT0;
		break;
	}
	case ERASE_CHIP: {
		bank1.fill(0xFF);
		bank2.fill(0xFF);
		eraseEnable = false;
		state = INIT0;
		break;
	}
	case ERASE_SECTOR: {
		// This shouldnt happen
		break;
	}
	case WRITE_SINGLE: {
		state = ACCEPT_WRITE;
		break;
	}
	case BANK_SWITCH: {
		state = ACCEPT_BANK_SWITCH;
		break;
	}
	}
}
