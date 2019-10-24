#include "registers.hpp"

namespace ARM7TDMI {

namespace SRFlag {
uint8_t get(const std::uint32_t& sr, const BitLocation& flag) {
  std::uint32_t mask = (1 << flag.size) - 1;
  return (sr >> flag.bit) & mask;
}

void set(uint32_t& sr, const BitLocation& flag, std::uint8_t val) {
  std::uint32_t mask = (1 << flag.size) - 1;
  if ((mask & val) != val) {
	// TODO: Complain about invalid value
	return;
  }
  mask = ~mask;
  sr &= mask;
  sr |= val;
}
}  // end  namespace SRFlag

ModeBank getModeBank(SRFlag::ModeBits modeBits) {
  switch (modeBits) {
	case SRFlag::ModeBits::USR:
	  return ModeBank::SYS;
	case SRFlag::ModeBits::FIQ:
	  return ModeBank::FIQ;
	case SRFlag::ModeBits::IRQ:
	  return ModeBank::IRQ;
	case SRFlag::ModeBits::SVC:
	  return ModeBank::SVC;
	case SRFlag::ModeBits::ABT:
	  return ModeBank::ABT;
	case SRFlag::ModeBits::UND:
	  return ModeBank::UND;
	case SRFlag::ModeBits::SYS:
	  return ModeBank::SYS;
	default:
	  // TODO: Log failure
	  exit(-1);
  }
}

std::uint32_t& RegisterSet::get(Register reg) {
  return get(currentBank, reg);
}

std::uint32_t& RegisterSet::get(ModeBank mode, Register reg) {
  if (reg <= Register::R12) {
	if (mode == ModeBank::FIQ && reg >= Register::R8) {
	  return registers.GPR_FIQ[reg - Register::R8];
	} else {
	  return registers.GPR[(uint8_t)reg];
	}
  } else if (reg == Register::R13) {
	return registers.SP[(uint8_t)mode];
  } else if (reg == Register::R14) {
	return registers.LR[(uint8_t)mode];
  } else if (reg == Register::R15) {
	return registers.PC;
  } else if (reg == Register::CPSR) {
	return registers.CPSR;
  } else if (reg == Register::SPSR) {
	if (mode == ModeBank::SYS) {
	  // TODO: Throw error
	}
	return registers.SPSR[(uint8_t)mode - 1];
  } else {
	// TODO: Log error
	exit(-1);
  }
}

bool RegisterSet::conditionCheck(Condition cond) {
  switch (cond) {
	case EQ:
	  return SRFlag::get(CPSR, SRFlag::z);
	case NE:
	  return !SRFlag::get(CPSR, SRFlag::z);
	case CS:
	  return SRFlag::get(CPSR, SRFlag::c);
	case CC:
	  return !SRFlag::get(CPSR, SRFlag::c);
	case MI:
	  return SRFlag::get(CPSR, SRFlag::n);
	case PL:
	  return !SRFlag::get(CPSR, SRFlag::n);
	case VS:
	  return SRFlag::get(CPSR, SRFlag::v);
	case VC:
	  return !SRFlag::get(CPSR, SRFlag::v);
	case HI:
	  return SRFlag::get(CPSR, SRFlag::c) && !SRFlag::get(CPSR, SRFlag::z);
	case LS:
	  return !SRFlag::get(CPSR, SRFlag::c) && SRFlag::get(CPSR, SRFlag::z);
	case GE:
	  return SRFlag::get(CPSR, SRFlag::n) == SRFlag::get(CPSR, SRFlag::v);
	case LT:
	  return SRFlag::get(CPSR, SRFlag::n) != SRFlag::get(CPSR, SRFlag::v);
	case GT:
	  return SRFlag::get(CPSR, SRFlag::n) == SRFlag::get(CPSR, SRFlag::v) &&
	         !SRFlag::get(CPSR, SRFlag::z);
	case LE:
	  return SRFlag::get(CPSR, SRFlag::n) != SRFlag::get(CPSR, SRFlag::v) &&
	         SRFlag::get(CPSR, SRFlag::z);
	case AL:
	  return true;
	case NV:
	  return false;
	default:  // TODO Complain
	  return false;
  }
}

void RegisterSet::switchMode(SRFlag::ModeBits mode) {
  auto newBank = getModeBank(mode);
  if (newBank == currentBank)
	return;

  if (newBank != ModeBank::SYS) {
	get(newBank, SPSR) = get(currentBank, CPSR);
  }
  currentBank = newBank;
  SRFlag::set(registers.CPSR, SRFlag::modeBits, mode);
}

}  // namespace ARM7TDMI
