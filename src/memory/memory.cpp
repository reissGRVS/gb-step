#include "memory/memory.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "memory/flash.hpp"
#include "memory/sram.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

Memory::Memory(std::shared_ptr<SystemClock> clock,
	std::string biosPath,
	std::string romPath,
	Joypad& joypad)
	: clock(clock)
	, joypad(joypad)
{
	// Read BIOS
	{
		std::ifstream infile(biosPath);
		if (!infile.is_open()) {
			spdlog::get("std")->error("BIOS not found at supplied path");
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.gen.bios.size()) {
			spdlog::get("std")->error("BIOS too big");
			exit(-1);
		}
		infile.read(reinterpret_cast<char*>(mem.gen.bios.data()), length);
	}
	// Read ROM
	{
		std::ifstream infile(romPath);
		if (!infile.is_open()) {
			spdlog::get("std")->error("ROM not found at supplied path");
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.ext.rom.size()) {
			spdlog::get("std")->error("ROM too big");
			exit(-1);
		}
		infile.read(reinterpret_cast<char*>(mem.ext.rom.data()), length);

		auto backupID = FindBackupID(length);

		if (backupID == FLASH1M_V) {
			mem.ext.backup = std::make_unique<Flash>(FlashSize::Double);
		} else if (backupID == FLASH512_V || backupID == FLASH_V) {
			mem.ext.backup = std::make_unique<Flash>(FlashSize::Single);
		} else if (backupID == SRAM_V) {
			mem.ext.backup = std::make_unique<SRAM>();
		} else {
			// TODO: Implement EEPROM and no backup
			spdlog::get("std")->error("Unsupported Backup type {}", backupID);
			mem.ext.backup = std::make_unique<SRAM>();
			//   exit(-1);
		}
	}
}

std::string Memory::FindBackupID(size_t length)
{
	for (std::uint32_t romAddr = 0u; romAddr < length; romAddr += 4) {
		for (const auto& idString : BACKUP_ID_STRINGS) {
			if (std::memcmp(idString.c_str(), mem.ext.rom.data() + romAddr,
					idString.size())
				== 0) {
				spdlog::get("std")->error("Backup type {} @ {:X}", idString, romAddr);
				return idString;
			}
		}
	}
	return "NONE";
}

uint32_t Memory::ReadToSize(std::uint8_t* byte, AccessSize size)
{
	auto word = reinterpret_cast<std::uint32_t*>(byte);
	return (*word) & size;
}

uint32_t Memory::Read(AccessSize size,
	std::uint32_t address,
	Sequentiality seq)
{
	auto page = address >> 24;

	if (address == KEYINPUT) {
		return joypad.getKeyStatus();
	}

	Tick(size, page, seq);
	switch (page) {
	case 0x00:
		if ((address & PAGE_MASK) < BIOS_SIZE) {
			return ReadToSize(&mem.gen.bios[address & BIOS_MASK], size);
		} else {
			spdlog::get("std")->error("Reading from Invalid BIOS memory");
			// exit(-1);
			return 0;
		}
	case 0x01: {
		spdlog::get("std")->error("Reading from Unused memory");
		break;
	}
	case 0x02:
		return ReadToSize(&mem.gen.wramb[address & WRAMB_MASK], size);
	case 0x03:
		return ReadToSize(&mem.gen.wramc[address & WRAMC_MASK], size);
	case 0x04:
		if (seq != Sequentiality::FREE) {
			spdlog::get("std")->debug("IORead {:X} size: {:X}", address,
				(uint32_t)size);
		}
		return ReadToSize(&mem.gen.ioreg[address & IOREG_MASK], size);
	case 0x05:
		return ReadToSize(&mem.disp.pram[address & PRAM_MASK], size);
	case 0x06:
		return ReadToSize(&mem.disp.vram[address & VRAM_MASK], size);
	case 0x07:
		return ReadToSize(&mem.disp.oam[address & OAM_MASK], size);
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D:
		return ReadToSize(&mem.ext.rom[address & ROM_MASK], size);
	case 0x0E:
		return mem.ext.backup->Read(address);
	default:
		break;
	}

	spdlog::get("std")->error("WTF IS THIS MEMORY READ??? Addr {:X}", address);
	PublishWriteCallback(1);
	return 0;
	// exit(-1);
}

void Memory::WriteToSize(std::uint8_t* byte,
	std::uint32_t value,
	AccessSize size)
{
	switch (size) {
	case AccessSize::Byte: {
		(*byte) = value;
		break;
	}
	case AccessSize::Half: {
		auto half = reinterpret_cast<std::uint16_t*>(byte);
		(*half) = value;
		break;
	}
	case AccessSize::Word: {
		auto word = reinterpret_cast<std::uint32_t*>(byte);
		(*word) = value;
		break;
	}
	}
}

void Memory::Write(AccessSize size,
	std::uint32_t address,
	std::uint32_t value,
	Sequentiality seq)
{
	auto page = address >> 24;
#ifndef NDEBUG
	PublishWriteCallback(address);
#endif
	Tick(size, page, seq);
	// TODO: Check if bus widths affect anything
	switch (page) {
	case 0x02:
		WriteToSize(&mem.gen.wramb[address & WRAMB_MASK], value, size);
		break;
	case 0x03:
		WriteToSize(&mem.gen.wramc[address & WRAMC_MASK], value, size);
		break;
	case 0x04: {
		if (seq == Sequentiality::FREE) {
			spdlog::get("std")->trace("IOWrite {:X} @ {:X} size: {:X} type: {:X}",
				value, address, (uint32_t)size, seq);
			WriteToSize(&mem.gen.ioreg[address & IOREG_MASK], value, size);
		} else {
			if (address < 0x40000E0 && address > 0x40000AF)
				spdlog::get("std")->debug("IOWrite {:X} @ {:X} size: {:X} type: {:X}",
					value, address, (uint32_t)size, seq);
			if (size == Half || size == Byte) {
				auto callback = ioCallbacks.find(address);
				if (callback != ioCallbacks.end()) {
					callback->second(value);
				} else {
					WriteToSize(&mem.gen.ioreg[address & IOREG_MASK], value, size);
				}
			}

			if (size == Word) {
				auto callback = ioCallbacks.find(address);
				if (callback != ioCallbacks.end()) {
					callback->second(value);
				} else {
					WriteToSize(&mem.gen.ioreg[address & IOREG_MASK], value, Half);
				}

				callback = ioCallbacks.find(address + 2);
				if (callback != ioCallbacks.end()) {
					callback->second(value >> 16);
				} else {
					WriteToSize(&mem.gen.ioreg[(address + 2) & IOREG_MASK], value >> 16,
						Half);
				}
			}
		}

		break;
	}
	case 0x05:
		WriteToSize(&mem.disp.pram[address & PRAM_MASK], value, size);
		break;
	case 0x06:
		WriteToSize(&mem.disp.vram[address & VRAM_MASK], value, size);
		break;
	case 0x07:
		WriteToSize(&mem.disp.oam[address & OAM_MASK], value, size);
		break;
	case 0x0E:
		mem.ext.backup->Write(address, value);
	default:
		break;
	}
}

void Memory::TickBySize(AccessSize size,
	std::uint32_t ticks8,
	std::uint32_t ticks16,
	std::uint32_t ticks32)
{
	switch (size) {
	case Byte:
		clock->Tick(ticks8);
		break;
	case Half:
		clock->Tick(ticks16);
		break;
	case Word:
		clock->Tick(ticks32);
		break;
	}
}

void Memory::Tick(AccessSize size, std::uint32_t page, Sequentiality seq)
{
	if (seq != NSEQ && seq != SEQ) {
		return;
	}
	// TODO: Plus 1 cycle if GBA accesses video memory at the same time. for OAM
	// PRAM VRAM
	switch (page) {
	case 0x00:
		// BIOS
		clock->Tick(1);
		break;
	case 0x01:
		// unused
		break;
	case 0x02:
		// WRAM 256 - 2 wait
		TickBySize(size, 3, 3, 6);
		break;
	case 0x03:
		// WRAM 32
		clock->Tick(1);
		break;
	case 0x04:
		// IO
		clock->Tick(1);
		break;
	case 0x05:
		// BG PRAM
		TickBySize(size, 1, 1, 2);
		break;
	case 0x06:
		// VRAM
		TickBySize(size, 1, 1, 2);
		break;
	case 0x07:
		// OAM
		clock->Tick(1);
		break;
	case 0x08:
	case 0x09: {
		// Game Pak ROM/FlashROM - WS0
		const std::array<uint8_t, 4> WS0_NSEQ = { 4, 3, 2, 8 };
		const std::array<uint8_t, 2> WS0_SEQ = { 2, 1 };
		auto waitCnt = GetHalf(WAITCNT);

		auto secondAccess = WS0_SEQ[BIT_RANGE(waitCnt, 4, 4)] + 1;
		auto firstAccess = (seq == SEQ) ? secondAccess : WS0_NSEQ[BIT_RANGE(waitCnt, 2, 3)] + 1;

		TickBySize(size, firstAccess, firstAccess, firstAccess + secondAccess);
		break;
	}
	case 0x0A:
	case 0x0B: {
		// Game Pak ROM/FlashROM - WS1
		const std::array<uint8_t, 4> WS1_NSEQ = { 4, 3, 2, 8 };
		const std::array<uint8_t, 2> WS1_SEQ = { 4, 1 };
		auto waitCnt = GetHalf(WAITCNT);

		auto secondAccess = WS1_SEQ[BIT_RANGE(waitCnt, 7, 7)] + 1;
		auto firstAccess = (seq == SEQ) ? secondAccess : WS1_NSEQ[BIT_RANGE(waitCnt, 5, 6)] + 1;

		TickBySize(size, firstAccess, firstAccess, firstAccess + secondAccess);
		break;
	}
	case 0x0C:
	case 0x0D: {
		// Game Pak ROM/FlashROM - WS2
		const std::array<uint8_t, 4> WS2_NSEQ = { 4, 3, 2, 8 };
		const std::array<uint8_t, 2> WS2_SEQ = { 8, 1 };
		auto waitCnt = GetHalf(WAITCNT);

		auto secondAccess = WS2_SEQ[BIT_RANGE(waitCnt, 10, 10)] + 1;
		auto firstAccess = (seq == SEQ) ? secondAccess : WS2_NSEQ[BIT_RANGE(waitCnt, 8, 9)] + 1;
		TickBySize(size, firstAccess, firstAccess, firstAccess + secondAccess);
		break;
	}
	case 0x0E: {
		// Game Pak ROM/FlashROM - WS2
		const std::array<uint8_t, 4> SRAM_NSEQ = { 4, 3, 2, 8 };
		auto waitCnt = GetHalf(WAITCNT);
		auto firstAccess = SRAM_NSEQ[BIT_RANGE(waitCnt, 0, 1)] + 1;
		clock->Tick(firstAccess);
		break;
	} break;
	default:
		break;
	}
}

void Memory::SetDebugWriteCallback(
	std::function<void(std::uint32_t)> callback)
{
	PublishWriteCallback = callback;
}

void Memory::SetIOWriteCallback(std::uint32_t address,
	std::function<void(std::uint32_t)> callback)
{
	ioCallbacks.emplace(address, callback);
}