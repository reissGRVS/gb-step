#include "memory.hpp"

#include <iostream>
#include <fstream>

#include "spdlog/spdlog.h"

Memory::Memory(std::string biosPath, std::string romPath)
{
	//Read BIOS
	{
		std::ifstream infile(biosPath);
		if (!infile.is_open())
		{
			spdlog::error("BIOS not found at supplied path");
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.gen.bios.size())
		{
			//TODO: Log bios too big
			spdlog::error("BIOS too big");
			exit(-1);
		}
		infile.read(reinterpret_cast<char *>(mem.gen.bios.data()), length);
	}
	//Read ROM
	{
		std::ifstream infile(romPath);
		if (!infile.is_open())
		{
			spdlog::error("ROM not found at supplied path");
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.ext.rom.size())
		{
			spdlog::error("ROM too big");
			exit(-1);
		}
		infile.read(reinterpret_cast<char *>(mem.ext.rom.data()), length);
	}
}

uint32_t Memory::ReadToSize(std::uint8_t *byte, AccessSize size)
{
	auto word = reinterpret_cast<std::uint32_t *>(byte);
	return (*word) & size;
}

uint32_t Memory::Read(AccessSize size, std::uint32_t address, Sequentiality accessType)
{
	auto page = address >> 24;
	//TODO: Take size into account
	//TODO: Increment cycles based on AccessType
	switch (page)
	{
	case 0x00:
		return ReadToSize(&mem.gen.bios[address & bios_mask], size);
	case 0x01:
		return 0; //not used
	case 0x02:
		return ReadToSize(&mem.gen.wramb[address & wramb_mask], size);
	case 0x03:
		return ReadToSize(&mem.gen.wramc[address & wramc_mask], size);
	case 0x04:
		return ReadToSize(&mem.gen.ioreg[address & ioreg_mask], size);
	case 0x05:
		return ReadToSize(&mem.disp.pram[address & pram_mask], size);
	case 0x06:
		return ReadToSize(&mem.disp.vram[address & vram_mask], size);
	case 0x07:
		return ReadToSize(&mem.disp.oam[address & oam_mask], size);
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D:
		return ReadToSize(&mem.ext.rom[address & rom_mask], size);
	default:
		return 0;
	}
}

void Memory::WriteToSize(std::uint8_t *byte, std::uint32_t value, AccessSize size)
{
	switch (size)
	{
	case AccessSize::Byte:
	{
		(*byte) = value;
		break;
	}
	case AccessSize::Half:
	{
		auto half = reinterpret_cast<std::uint16_t *>(byte);
		(*half) = value;
		break;
	}
	case AccessSize::Word:
	{
		auto word = reinterpret_cast<std::uint32_t *>(byte);
		(*word) = value;
		break;
	}
	}
}

void Memory::Write(AccessSize size, std::uint32_t address, std::uint32_t value, Sequentiality accessType)
{
	auto page = address >> 24;

	switch (page)
	{
	case 0x02:
		WriteToSize(&mem.gen.wramb[address & wramb_mask], value, size);
		break;
	case 0x03:
		WriteToSize(&mem.gen.wramc[address & wramc_mask], value, size);
		break;
	case 0x04:
		WriteToSize(&mem.gen.ioreg[address & ioreg_mask], value, size);
		break;
	case 0x05:
		WriteToSize(&mem.disp.pram[address & pram_mask], value, size);
		break;
	case 0x06:
		WriteToSize(&mem.disp.vram[address & vram_mask], value, size);
		break;
	case 0x07:
		WriteToSize(&mem.disp.oam[address & oam_mask], value, size);
		break;
	default:
		break;
	}
}