#include "memory/memory.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "memory/flash.hpp"
#include "memory/sram.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

Memory::Memory(std::shared_ptr<SystemClock> clock, std::string biosPath,
               std::string romPath, Joypad &joypad)
    : clock(clock), joypad(joypad) {
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
    infile.read(reinterpret_cast<char *>(mem.gen.bios.data()), length);
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
    infile.read(reinterpret_cast<char *>(mem.ext.rom.data()), length);

    auto backupID = FindBackupID(length);

    auto extensionIndex = romPath.find_last_of(".");
    auto saveFilePath = romPath;
    if (extensionIndex != std::string::npos) {
      saveFilePath = saveFilePath.substr(0, extensionIndex);
    }
    saveFilePath = saveFilePath + ".gbasav";

    if (backupID == FLASH1M_V) {
      mem.ext.backup = std::make_unique<Flash>(FlashSize::Double, saveFilePath);
    } else if (backupID == FLASH512_V || backupID == FLASH_V) {
      mem.ext.backup = std::make_unique<Flash>(FlashSize::Single, saveFilePath);
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

void Memory::AttachIORegisters(std::shared_ptr<IORegisters> io) {
  mem.gen.io = io;
}

void Memory::AttachIRIORegisters(std::shared_ptr<IRIORegisters> irio_) {
  irio = irio_;
}

std::string Memory::FindBackupID(size_t length) {
  for (U32 romAddr = 0u; romAddr < length; romAddr += 4) {
    for (const auto &idString : BACKUP_ID_STRINGS) {
      if (std::memcmp(idString.c_str(), mem.ext.rom.data() + romAddr,
                      idString.size()) == 0) {
        spdlog::get("std")->error("Backup type {} @ {:X}", idString, romAddr);
        return idString;
      }
    }
  }
  return "NONE";
}

uint32_t Memory::Read(AccessSize size, U32 address, Sequentiality seq) {
  auto page = address >> 24;

  if (address == KEYINPUT) {
    return joypad.getKeyStatus();
  }

  Tick(size, page, seq);
  switch (page) {
  case 0x00:
    if ((address & PAGE_MASK) < BIOS_SIZE) {
      return ReadToSize(mem.gen.bios, address & BIOS_MASK, size);
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
    return ReadToSize(mem.gen.wramb, address & WRAMB_MASK, size);
  case 0x03:
    return ReadToSize(mem.gen.wramc, address & WRAMC_MASK, size);
  case 0x04: {
    return mem.gen.io->Read(size, address, seq);
  }
  case 0x05:
    return ReadToSize(mem.disp.pram, address & PRAM_MASK, size);
  case 0x06:
    return ReadToSize(mem.disp.vram, address & VRAM_MASK, size);
  case 0x07:
    return ReadToSize(mem.disp.oam, address & OAM_MASK, size);
  case 0x08:
  case 0x09:
  case 0x0A:
  case 0x0B:
  case 0x0C:
  case 0x0D:
    return ReadToSize(mem.ext.rom, address & ROM_MASK, size);
  case 0x0E:
    return mem.ext.backup->Read(address);
  default:
    break;
  }

  spdlog::get("std")->error("WTF IS THIS MEMORY READ??? Addr {:X}", address);
  PublishWriteCallback(1);
  return 0;
}

void Memory::Write(AccessSize size, U32 address, U32 value, Sequentiality seq) {
  auto page = address >> 24;
#ifndef NDEBUG
  PublishWriteCallback(address);
#endif
  Tick(size, page, seq);
  // TODO: Check if bus widths affect anything
  switch (page) {
  case 0x02:
    WriteToSize(mem.gen.wramb, address & WRAMB_MASK, value, size);
    break;
  case 0x03:
    WriteToSize(mem.gen.wramc, address & WRAMC_MASK, value, size);
    break;
  case 0x04:
    mem.gen.io->Write(size, address, value, seq);
    break;
  case 0x05:
    WriteToSize(mem.disp.pram, address & PRAM_MASK, value, size);
    break;
  case 0x06:
    WriteToSize(mem.disp.vram, address & VRAM_MASK, value, size);
    break;
  case 0x07:
    WriteToSize(mem.disp.oam, address & OAM_MASK, value, size);
    break;
  case 0x0E:
    mem.ext.backup->Write(address, value);
  default:
    break;
  }
}

void Memory::TickBySize(AccessSize size, U32 ticks8, U32 ticks16, U32 ticks32) {
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

void Memory::Tick(AccessSize size, U32 page, Sequentiality seq) {
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
    auto[nseqTicks, seqTicks] =
        irio->GetWaitstateTicks(IRIORegisters::Waitstate::WS0);
    auto firstAccess = (seq == SEQ) ? seqTicks : nseqTicks;
    TickBySize(size, firstAccess, firstAccess, firstAccess + seqTicks);
    break;
  }
  case 0x0A:
  case 0x0B: {
    // Game Pak ROM/FlashROM - WS1
    auto[nseqTicks, seqTicks] =
        irio->GetWaitstateTicks(IRIORegisters::Waitstate::WS1);
    auto firstAccess = (seq == SEQ) ? seqTicks : nseqTicks;
    TickBySize(size, firstAccess, firstAccess, firstAccess + seqTicks);
    break;
  }
  case 0x0C:
  case 0x0D: {
    // Game Pak ROM/FlashROM - WS2
    auto[nseqTicks, seqTicks] =
        irio->GetWaitstateTicks(IRIORegisters::Waitstate::WS2);
    auto firstAccess = (seq == SEQ) ? seqTicks : nseqTicks;
    TickBySize(size, firstAccess, firstAccess, firstAccess + seqTicks);
    break;
  }
  case 0x0E: {
    // Game Pak ROM/FlashROM
    auto nseqTicks =
        irio->GetWaitstateTicks(IRIORegisters::Waitstate::WS2).nseq;
    clock->Tick(nseqTicks);
    break;
  } break;
  default:
    break;
  }
}

void Memory::SetDebugWriteCallback(std::function<void(U32)> callback) {
  PublishWriteCallback = callback;
}

void Memory::SetIOWriteCallback(U32 address,
                                std::function<void(U32)> callback) {
  ioCallbacks.emplace(address, callback);
}

U8 Memory::GetByte(const U32 &address) {
  return Read(AccessSize::Byte, address, Sequentiality::FREE);
}

U16 Memory::GetHalf(const U32 &address) {
  return Read(AccessSize::Half, address, Sequentiality::FREE);
}

U32 Memory::GetWord(const U32 &address) {
  return Read(AccessSize::Word, address, Sequentiality::FREE);
}

void Memory::SetByte(const U32 &address, const U8 &value) {
  Write(AccessSize::Byte, address, value, Sequentiality::FREE);
}

void Memory::SetHalf(const U32 &address, const U16 &value) {
  Write(AccessSize::Half, address, value, Sequentiality::FREE);
}

void Memory::SetWord(const U32 &address, const U32 &value) {
  Write(AccessSize::Word, address, value, Sequentiality::FREE);
}
