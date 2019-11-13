#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "../memory.hpp"

namespace ARM7TDMI {
// https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
enum Condition {
  EQ = 0x0,
  NE = 0x1,
  CS = 0x2,
  CC = 0x3,
  MI = 0x4,
  PL = 0x5,
  VS = 0x6,
  VC = 0x7,
  HI = 0x8,
  LS = 0x9,
  GE = 0xA,
  LT = 0xB,
  GT = 0xC,
  LE = 0xD,
  AL = 0xE,
  NV = 0xF
};

// https://problemkaputt.de/gbatek.htm#armcpuregisterset
enum Register {
  R0 = 0,
  R1 = 1,
  R2 = 2,
  R3 = 3,
  R4 = 4,
  R5 = 5,
  R6 = 6,
  R7 = 7,
  R8 = 8,
  R9 = 9,
  R10 = 10,
  R11 = 11,
  R12 = 12,
  R13 = 13,
  R14 = 14,
  R15 = 15,
  CPSR = 16,
  SPSR = 17
};

namespace SRFlag {
struct BitLocation {
  const std::uint8_t bit;
  const std::uint8_t size;
};

enum ModeBits {
  USR = 0x10,
  FIQ = 0x11,
  IRQ = 0x12,
  SVC = 0x13,
  ABT = 0x17,
  UND = 0x1B,
  SYS = 0x1F
};

const BitLocation modeBits = {0, 5};
const BitLocation thumb = {5, 1};
const BitLocation fiqDisable = {6, 1};
const BitLocation irqDisable = {7, 1};
const BitLocation q = {27, 1};
const BitLocation flags = {28, 4};
const BitLocation v = {28, 1};
const BitLocation c = {29, 1};
const BitLocation z = {30, 1};
const BitLocation n = {31, 1};

uint8_t get(const std::uint32_t& sr, const BitLocation& flag);
void set(std::uint32_t& sr, const BitLocation& flag, std::uint8_t val);
}  // namespace SRFlag

enum ModeBank {
  SYS = 0,
  FIQ = 1,
  SVC = 2,
  ABT = 3,
  IRQ = 4,
  UND = 5,
};

ModeBank getModeBank(SRFlag::ModeBits modeBits);

class RegisterSet {
 public:
  std::uint32_t view(Register reg);
  std::uint32_t& get(Register reg);

  std::uint32_t& get(ModeBank mode, Register reg);

  // https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
  bool conditionCheck(Condition cond);

  void switchMode(SRFlag::ModeBits mode);

 private:
  ModeBank currentBank = ModeBank::SYS;

  struct Registers {
	std::array<std::uint32_t, 13> GPR{};     // R0-12
	std::array<std::uint32_t, 5> GPR_FIQ{};  // R8-12 FIQ
	std::array<std::uint32_t, 6> SP{};       // R13 All modes
	std::array<std::uint32_t, 6> LR{};       // R14 All modes
	std::uint32_t PC{};                      // R15
	std::uint32_t CPSR{0x1F};
	std::array<std::uint32_t, 5> SPSR{};  // System/Usr mode has no SPSR
  } registers;
};

}  // namespace ARM7TDMI