#include "memory.hpp"
#include <unistd.h>
#include <fstream>
#include <iostream>

#include "spdlog/spdlog.h"
#include "utils.hpp"

Memory::Memory(std::string biosPath, std::string romPath, Joypad& joypad)
    : joypad(joypad) {
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
  }
}

uint32_t Memory::ReadToSize(std::uint8_t* byte, AccessSize size) {
  auto word = reinterpret_cast<std::uint32_t*>(byte);
  return (*word) & size;
}

uint32_t Memory::Read(AccessSize size, std::uint32_t address, Sequentiality) {
  auto page = address >> 24;

  if (address == KEYINPUT) {
	return joypad.getKeyStatus();
  }

  // TODO: Increment cycles based on AccessType
  switch (page) {
	case 0x00:
	  if ((address & PAGE_MASK) < BIOS_SIZE) {
		return ReadToSize(&mem.gen.bios[address & BIOS_MASK], size);
	  } else {
		spdlog::get("std")->error("Reading from Invalid BIOS memory");
		exit(-1);
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
	default:
	  break;
  }

  spdlog::get("std")->error("WTF IS THIS MEMORY READ??? Addr {:X}", address);
  exit(-1);
}

void Memory::WriteToSize(std::uint8_t* byte,
                         std::uint32_t value,
                         AccessSize size) {
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
                   Sequentiality) {
  auto page = address >> 24;
#ifndef NDEBUG
  PublishWriteCallback(address);
#endif

  switch (page) {
	case 0x02:
	  WriteToSize(&mem.gen.wramb[address & WRAMB_MASK], value, size);
	  break;
	case 0x03:
	  WriteToSize(&mem.gen.wramc[address & WRAMC_MASK], value, size);
	  break;
	case 0x04: {
	  WriteToSize(&mem.gen.ioreg[address & IOREG_MASK], value, size);
	  auto callback = ioCallbacks.find(address);
	  // TODO: Make sure that word writes call all half callbacks
	  if (callback != ioCallbacks.end()) {
		callback->second();
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
	default:
	  break;
  }
}

void Memory::SetDebugWriteCallback(
    std::function<void(std::uint32_t)> callback) {
  PublishWriteCallback = callback;
}

void Memory::SetIOWriteCallback(std::uint32_t address,
                                std::function<void()> callback) {
  ioCallbacks.emplace(address, callback);
}