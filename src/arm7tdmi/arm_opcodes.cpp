#include <unistd.h>
#include "arm7tdmi/cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace ARM7TDMI {

const ParamSegments DataProcessingSegments
    //  I         Opcode    S         Rn        Rd        Op2
    = {{25, 25}, {24, 21}, {20, 20}, {19, 16}, {15, 12}, {11, 0}};
const ParamSegments MultiplySegments
    //  A         S         Rd        Rn        Rs      Rm
    = {{21, 21}, {20, 20}, {19, 16}, {15, 12}, {11, 8}, {3, 0}};
const ParamSegments MultiplyLongSegments
    //  U       A       S       RdHi    RdLo    Rn     Rm
    = {{22, 22}, {21, 21}, {20, 20}, {19, 16}, {15, 12}, {11, 8}, {3, 0}};
const ParamSegments SingleDataSwapSegments
    //  B       Rn      Rd      Rm
    = {{22, 22}, {19, 16}, {15, 12}, {3, 0}};
const ParamSegments BranchAndExchangeSegments
    //  Rn
    = {{3, 0}};
const ParamSegments HalfwordDTRegOffsetSegments
    //  P         U         W         L         Rn       Rd      S     H   Rm
    = {{24, 24}, {23, 23}, {21, 21}, {20, 20}, {19, 16},
       {15, 12}, {6, 6},   {5, 5},   {3, 0}};
const ParamSegments HalfwordDTImmOffsetSegments
    //  P       U       W       L       Rn      Rd      Offset S     H Offset
    = {{24, 24}, {23, 23}, {21, 21}, {20, 20}, {19, 16},
       {15, 12}, {11, 8},  {6, 6},   {5, 5},   {3, 0}};
const ParamSegments SingleDataTransferSegments
    //  I       P       U       B       W       L       Rn      Rd      Offset
    = {{25, 25}, {24, 24}, {23, 23}, {22, 22}, {21, 21},
       {20, 20}, {19, 16}, {15, 12}, {11, 0}};
const ParamSegments BlockDataTransferSegments
    //  P       U       S       W       L       Rn      RegList
    = {{24, 24}, {23, 23}, {22, 22}, {21, 21}, {20, 20}, {19, 16}, {15, 0}};

const ParamSegments BranchSegments{{24, 24}, {23, 0}};

void CPU::Shift(std::uint32_t& value,
                std::uint32_t amount,
                const std::uint32_t& shiftType,
                std::uint8_t& carryOut,
                bool regProvidedAmount) {
  switch (shiftType) {
	case 0b00:  // LSL
	{
	  if (amount) {
		value <<= (amount - 1);
		carryOut = value >> 31;
		value <<= 1;
	  }
	  break;
	}
	case 0b01:  // LSR
	case 0b10:  // ASR
	{
	  auto neg = value >> 31;

	  if (amount > 32) {
		if (neg && shiftType == 0b10) {
		  value = NBIT_MASK(32);
		  carryOut = 1;
		} else {
		  value = 0;
		  carryOut = 0;
		}
	  }

	  if (amount == 0) {
		if (regProvidedAmount) {
		  spdlog::get("std")->trace("Reg provided amount");
		  return;
		} else {
		  amount = 32;
		}
	  }

	  value >>= (amount - 1);
	  carryOut = value & 1;
	  value >>= 1;
	  if (neg && shiftType == 0b10) {
		std::uint32_t mask = (NBIT_MASK(amount) << (32 - amount));
		value |= mask;
	  }
	  break;
	}
	case 0b11:  // RR
	{
	  if (amount > 0u) {
		amount = amount % 32;
		if (amount == 0)
		  amount = 32;
	  }

	  // RRX
	  if (amount == 0 && !regProvidedAmount) {
		auto carryCopy = carryOut;
		carryOut = value & 1;
		value >>= 1;
		if (carryCopy) {
		  value |= 1 << 31;
		}
	  } else if (regProvidedAmount && amount == 0) {
		break;
	  } else {
		auto topHalf = value << (32 - amount);
		value >>= (amount - 1);
		carryOut = value & 1;
		value >>= 1;
		value |= topHalf;
	  }
	  break;
	}
	default:  // Should not be possible
	{
	  spdlog::get("std")->error("CPU::Shift invalid parameters");
	  break;
	}
  }
}

std::function<void()> CPU::ArmOperation(OpCode opcode) {
  if (!registers.conditionCheck((Condition)(opcode >> 28))) {
	return []() {
	  spdlog::get("std")->trace("Condition failed");
	  return;
	};
  }

  switch (BIT_RANGE(opcode, 26, 27)) {
	case 0b00: {
	  if (opcode & (1 << 25)) {
		return std::bind(&CPU::ArmDataProcessing_P, this,
		                 ParseParams(opcode, DataProcessingSegments));
	  } else if ((opcode & 0xFFFFFF0) == 0x12FFF10) {
		return std::bind(&CPU::ArmBranchAndExchange_P, this,
		                 ParseParams(opcode, BranchAndExchangeSegments));
	  } else if ((opcode & 0x18000F0) == 0x0000090) {
		return std::bind(&CPU::ArmMultiply_P, this,
		                 ParseParams(opcode, MultiplySegments));
	  } else if ((opcode & 0x18000F0) == 0x0800090) {
		return std::bind(&CPU::ArmMultiplyLong_P, this,
		                 ParseParams(opcode, MultiplyLongSegments));
	  } else if ((opcode & 0x1B00FF0) == 0x1000090) {
		return std::bind(&CPU::ArmSingleDataSwap_P, this,
		                 ParseParams(opcode, SingleDataSwapSegments));
	  } else if ((opcode & 0xF0) == 0xB0 || (opcode & 0xF0) == 0xD0 ||
	             (opcode & 0xF0) == 0xF0) {
		if (opcode & (1 << 22)) {
		  return std::bind(&CPU::ArmHalfwordDTImmOffset_P, this,
		                   ParseParams(opcode, HalfwordDTImmOffsetSegments));
		} else {
		  return std::bind(&CPU::ArmHalfwordDTRegOffset_P, this,
		                   ParseParams(opcode, HalfwordDTRegOffsetSegments));
		}
	  } else {
		return std::bind(&CPU::ArmDataProcessing_P, this,
		                 ParseParams(opcode, DataProcessingSegments));
	  }
	}
	case 0b01:  // SDT and Undef
	{
	  std::uint32_t undefMask = (0b11 << 25) | (0b1 << 4);

	  if ((opcode & undefMask) == undefMask) {
		return std::bind(&CPU::ArmUndefined_P, this, ParamList());
	  } else {
		return std::bind(&CPU::ArmSingleDataTransfer_P, this,
		                 ParseParams(opcode, SingleDataTransferSegments));
	  }
	}
	case 0b10:  // BDT and Branch
	{
	  if (opcode & (1 << 25)) {
		return std::bind(&CPU::ArmBranch_P, this,
		                 ParseParams(opcode, BranchSegments));
	  } else {
		return std::bind(&CPU::ArmBlockDataTransfer_P, this,
		                 ParseParams(opcode, BlockDataTransferSegments));
	  }
	}
	case 0b11:  // CoProc and SWI
	{
	  const auto swiMask = 0xF000000;
	  if ((swiMask & opcode) == swiMask) {
		return std::bind(&CPU::ArmSWI_P, this, ParamList());
	  } else {
		spdlog::get("std")->error("Attempting CoProc???");
		exit(-1);
	  }
	}
	default: {
	  spdlog::get("std")->error("This should be impossible");
	  exit(-1);
	}
  }
}
void CPU::ArmMRS(bool Ps, std::uint8_t Rd) {
  auto& dest = registers.get((Register)Rd);
  if (Ps) {
	dest = registers.get(Register::SPSR);
	spdlog::get("std")->trace("	MRS SPSR");
  } else {
	dest = registers.get(Register::CPSR);
	spdlog::get("std")->trace("	MRS CPSR {:X}", dest);
  }
}

void CPU::ArmMSR(bool I, bool Pd, bool flagsOnly, std::uint16_t source) {
  auto value = registers.get((Register)(source & NBIT_MASK(4)));
  if (!flagsOnly) {
	if (Pd) {
	  registers.get(Register::SPSR) = value;
	  spdlog::get("std")->trace("	MSR SPSR {:X}", value);
	} else {
	  registers.get(Register::CPSR) = value;
	  spdlog::get("std")->trace("	MSR CPSR {:X}", value);
	  registers.switchMode((SRFlag::ModeBits)(value & NBIT_MASK(5)));
	}
  } else {
	if (I) {
	  value = (source & NBIT_MASK(8));
	  auto rotate = (source >> 8) * 2;
	  const std::uint32_t ROR = 0b11;
	  // Maybe this should do RRX as well?
	  uint8_t carry = 0;
	  if (rotate) {
		Shift(value, rotate, ROR, carry, false);
	  }
	}
	value >>= 28;

	if (Pd) {
	  SRFlag::set(registers.get(Register::SPSR), SRFlag::flags, value);
	  spdlog::get("std")->trace("	MSR SPSR_f {:X}", value);
	} else {
	  SRFlag::set(registers.get(Register::CPSR), SRFlag::flags, value);
	  spdlog::get("std")->trace("	MSR CPSR_f {:X}", value);
	}
  }
}

void CPU::ArmDataProcessing_P(ParamList params) {
  const std::uint32_t Op2 = params[0], Rd = params[1], Rn = params[2],
                      S = params[3], OpCode = params[4], I = params[5];
  ArmDataProcessing(I, OpCode, S, Rn, Rd, Op2);
}

void CPU::ArmDataProcessing(std::uint32_t I,
                            std::uint32_t OpCode,
                            std::uint32_t S,
                            std::uint32_t Rn,
                            std::uint32_t Rd,
                            std::uint32_t Op2) {
  spdlog::get("std")->trace(
      "DP I:{:X} OpCode:{:X} S:{:X} Rn:{:X} Rd:{:X} Op2:{:X}", I, OpCode, S, Rn,
      Rd, Op2);

  // PSR Transfers
  if (!S && OpCode >= DPOps::TST && OpCode <= DPOps::CMN) {
	bool P = BIT_RANGE(OpCode, 1, 1);
	if (Rn == 0xF) {
	  ArmMRS(P, Rd);
	} else {
	  bool flagsOnly = !(Rn & NBIT_MASK(1));
	  ArmMSR(I, P, flagsOnly, Op2);
	}
	return;
  }

  std::uint32_t Op1Val = registers.get((Register)Rn);

  // If using PC, bit 1 is cleared
  if (Rn == 15) {
	BIT_CLEAR(Op1Val, 1);
  }

  auto& dest = registers.get((Register)Rd);
  auto carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
  std::uint32_t Op2Val = 0;

  if (!I) {
	auto Rm = registers.get((Register)(Op2 & NBIT_MASK(4)));
	auto shiftType = BIT_RANGE(Op2, 5, 6);
	auto shiftAmount = BIT_RANGE(Op2, 7, 11);

	if (BIT_RANGE(Op2, 4, 4))  // Shift amount from register
	{
	  if (BIT_RANGE(Op2, 7, 7)) {
		spdlog::get("std")->error(
		    "This instruction should have been UNDEF or MUL");
	  }
	  // If also using R15 to specify shift + 4 to Rm
	  if ((Op2 & NBIT_MASK(4)) == 15) {
		spdlog::get("std")->trace("PC: {:X}", Rm);
		Rm += 4;
	  }

	  clock->Tick(1);
	  shiftAmount = registers.get((Register)(shiftAmount >> 1)) & NBIT_MASK(8);
	  Shift(Rm, shiftAmount, shiftType, carry, true);
	} else {
	  Shift(Rm, shiftAmount, shiftType, carry, false);
	}

	Op2Val = Rm;
  } else {
	auto Imm = Op2 & NBIT_MASK(8);
	auto rotate = (Op2 >> 8) * 2;
	const std::uint32_t ROR = 0b11;
	// Maybe this should do RRX as well?
	if (rotate) {
	  Shift(Imm, rotate, ROR, carry, false);
	}

	Op2Val = Imm;
  }

  auto SetFlags = [this](const std::uint32_t& S, const std::uint32_t& result,
                         const std::uint8_t& carry) {
	if (S) {
	  auto& cpsr = registers.get(CPSR);

	  std::uint8_t zVal = result == 0;
	  std::uint8_t nVal = BIT_RANGE(result, 31, 31);
	  SRFlag::set(cpsr, SRFlag::n, nVal);
	  SRFlag::set(cpsr, SRFlag::z, zVal);
	  SRFlag::set(cpsr, SRFlag::c, carry);
	  spdlog::get("std")->trace("Carry set to: {}", carry);
	}
  };

  if (Rd == 15 && S) {
	auto previousSPSR = registers.get(SPSR);
	registers.switchMode(
	    (SRFlag::ModeBits)SRFlag::get(registers.get(SPSR), SRFlag::modeBits));
	registers.get(CPSR) = previousSPSR;
	S = 0;
  }

  auto& cpsr = registers.get(CPSR);

  switch ((DPOps)OpCode) {
	case DPOps::AND: {
	  dest = Op1Val & Op2Val;
	  spdlog::get("std")->trace("	AND {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::EOR: {
	  dest = Op1Val ^ Op2Val;
	  spdlog::get("std")->trace("	EOR {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::SUB: {
	  auto result = (std::uint64_t)Op1Val - Op2Val;
	  dest = (uint32_t)result;
	  spdlog::get("std")->trace("	SUB {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  carry = Op1Val >= Op2Val;
	  auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}
	case DPOps::RSB: {
	  auto result = (std::uint64_t)Op2Val - Op1Val;
	  dest = (uint32_t)result;
	  spdlog::get("std")->trace("	RSB {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  carry = Op2Val >= Op1Val;
	  auto overflow = ((Op1Val ^ Op2Val) & ~(Op1Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::ADD: {
	  auto result = (std::uint64_t)Op1Val + Op2Val;
	  dest = (uint32_t)result;
	  spdlog::get("std")->trace("	ADD {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  carry = result >> 32;
	  auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::ADC: {
	  carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
	  auto result = (std::uint64_t)Op1Val + Op2Val + carry;
	  dest = (uint32_t)result;
	  spdlog::get("std")->trace("	ADC {:X} {:X} {:X} {:X}", Op1Val, Op2Val,
	                            carry, dest);
	  auto overflow = ((~(Op1Val ^ Op2Val) & ((Op1Val + Op2Val) ^ Op2Val)) ^
	                   (~((Op1Val + Op2Val) ^ carry) & (dest ^ carry))) >>
	                  31;
	  carry = result >> 32;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::SBC: {
	  carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
	  std::uint32_t Op3Val = carry ^ 1;
	  dest = Op1Val - Op2Val - Op3Val;
	  spdlog::get("std")->trace("	SBC {:X} {:X} {:X} {:X}", Op1Val, Op2Val,
	                            Op3Val, dest);
	  carry = (Op1Val >= Op2Val) && ((Op1Val - Op2Val) >= (Op3Val));
	  auto overflow = (((Op1Val ^ Op2Val) & ~((Op1Val - Op2Val) ^ Op2Val)) ^
	                   ((Op1Val - Op2Val) & ~dest)) >>
	                  31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::RSC: {
	  carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
	  std::uint32_t Op3Val = carry ^ 1;
	  dest = Op2Val - Op1Val - Op3Val;
	  spdlog::get("std")->trace("	RSC {:X} {:X} {:X} {:X}", Op1Val, Op2Val,
	                            Op3Val, dest);
	  carry = (Op2Val >= Op1Val) && ((Op2Val - Op1Val) >= (Op3Val));
	  auto overflow = (((Op2Val ^ Op1Val) & ~((Op2Val - Op1Val) ^ Op1Val)) ^
	                   ((Op2Val - Op1Val) & ~dest)) >>
	                  31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::TST: {
	  auto result = Op1Val & Op2Val;
	  spdlog::get("std")->trace("	TST {:X} {:X} {:X}", Op1Val, Op2Val,
	                            result);
	  SetFlags(1, result, carry);
	  S = 0;  // To not trigger SetFlags at bottom
	  break;
	}

	case DPOps::TEQ: {
	  auto result = Op1Val ^ Op2Val;
	  spdlog::get("std")->trace("	TEQ {:X} {:X} {:X}", Op1Val, Op2Val,
	                            result);
	  SetFlags(1, result, carry);
	  S = 0;
	  break;
	}

	case DPOps::CMP: {
	  auto result = (std::uint64_t)Op1Val - Op2Val;
	  spdlog::get("std")->trace("	CMP {:X} {:X} {:X}", Op1Val, Op2Val,
	                            result);
	  carry = Op1Val >= Op2Val;
	  SetFlags(1, result, carry);
	  auto overflow =
	      ((Op1Val ^ Op2Val) & ~(Op2Val ^ ((std::uint32_t)result))) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  spdlog::get("std")->trace("	V set to: {:X}", overflow);
	  S = 0;
	  break;
	}

	case DPOps::CMN: {
	  auto resultBig = (std::uint64_t)Op1Val + Op2Val;
	  std::uint32_t result = (uint32_t)resultBig;
	  spdlog::get("std")->trace("	CMN {:X} {:X} {:X}", Op1Val, Op2Val,
	                            result);
	  carry = resultBig >> 32;
	  SetFlags(1, result, carry);
	  auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ result)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  S = 0;
	  break;
	}

	case DPOps::ORR: {
	  dest = Op1Val | Op2Val;
	  spdlog::get("std")->trace("	ORR {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::MOV: {
	  dest = Op2Val;
	  spdlog::get("std")->trace("	MOV {:X}", dest);
	  break;
	}

	case DPOps::BIC: {
	  dest = Op1Val & ~Op2Val;
	  spdlog::get("std")->trace("	BIC {:X} {:X} {:X}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::MVN: {
	  dest = ~Op2Val;
	  spdlog::get("std")->trace("	MVN {:X}", dest);
	  break;
	}

	default:
	  break;
  }

  if (Rd == 15) {
	PipelineFlush();
  }

  SetFlags(S, dest, carry);
}

void CPU::ICyclesMultiply(const std::uint32_t& mulop) {
  auto ticks = 1u;
  auto mask = 0xFFFFFFFF;
  for (auto i = 0u; i < 3u; i++) {
	mask <<= 8;
	auto mulopsection = mask & mulop;
	if (mulopsection == 0 || mulopsection == mask) {
	  break;
	} else {
	  ticks++;
	}
  }
  clock->Tick(ticks);
}

void CPU::ArmMultiply_P(ParamList params) {
  spdlog::get("std")->trace("MUL");
  const std::uint32_t Rm = params[0], Rs = params[1], Rn = params[2],
                      Rd = params[3], S = params[4], A = params[5];
  ArmMultiply(A, S, Rd, Rn, Rs, Rm);
}

void CPU::ArmMultiply(std::uint32_t A,
                      std::uint32_t S,
                      std::uint32_t Rd,
                      std::uint32_t Rn,
                      std::uint32_t Rs,
                      std::uint32_t Rm) {
  auto& dest = registers.get((Register)Rd);

  if (Rd == Rm) {
	spdlog::get("std")->error("Invalid Arm Mult Rd == Rm");
	return;
  }

  if (Rm == 15 || Rs == 15 || Rn == 15 || Rd == 15) {
	spdlog::get("std")->error("Invalid Arm Mult Using PC");
	return;
  }

  std::int32_t op1 = registers.get((Register)Rm);
  std::int32_t op2 = registers.get((Register)Rs);
  ICyclesMultiply(op2);
  std::int32_t op3 = registers.get((Register)Rn);

  if (A) {
	clock->Tick(1);
	dest = op1 * op2 + op3;
  } else {
	dest = op1 * op2;
  }

  if (S) {
	auto& cpsr = registers.get(CPSR);
	std::uint8_t zVal = dest == 0;
	std::uint8_t nVal = dest >> 31;
	SRFlag::set(cpsr, SRFlag::n, nVal);
	SRFlag::set(cpsr, SRFlag::z, zVal);
  }
}

void CPU::ArmMultiplyLong_P(ParamList params) {
  spdlog::get("std")->trace("MULLONG");
  const std::uint32_t Rm = params[0], Rs = params[1], RdLo = params[2],
                      RdHi = params[3], S = params[4], A = params[5],
                      U = params[6];
  ArmMultiplyLong(U, A, S, RdHi, RdLo, Rs, Rm);
}

void CPU::ArmMultiplyLong(std::uint32_t U,
                          std::uint32_t A,
                          std::uint32_t S,
                          std::uint32_t RdHi,
                          std::uint32_t RdLo,
                          std::uint32_t Rs,
                          std::uint32_t Rm) {
  spdlog::get("std")->trace(
      "MULLONG Rm {} Rs {} RdHi {} RdLo {} U {} A {} S {}", Rm, Rs, RdHi, RdLo,
      U, A, S);
  if (RdLo == Rm || RdHi == Rm || RdLo == RdHi) {
	spdlog::get("std")->error("Invalid Arm Mult Long Rd == Rm");
	return;
  }

  if (Rm == 15 || Rs == 15 || RdHi == 15 || RdLo == 15) {
	spdlog::get("std")->error("Invalid Arm Mult Long Using PC");
	return;
  }

  clock->Tick(1);
  std::int64_t result = 0;
  std::uint32_t op1 = registers.get((Register)Rm);
  std::uint32_t op2 = registers.get((Register)Rs);
  ICyclesMultiply(op2);
  std::uint32_t op3 = registers.get((Register)RdLo);
  std::uint32_t op4 = registers.get((Register)RdHi);

  if (A) {
	clock->Tick(1);
	result = op4;
	result <<= 32;
	result += op3;
  }

  if (!U) {
	// UNSIGNED
	result += (uint64_t)op1 * op2;
  } else {
	// SIGNED
	int32_t signed_op1 = op1;
	int32_t signed_op2 = op2;
	result += (int64_t)signed_op1 * signed_op2;
  }

  registers.get((Register)RdLo) = (uint32_t)result;
  registers.get((Register)RdHi) = result >> 32;

  if (S) {
	auto& cpsr = registers.get(CPSR);
	std::uint8_t zVal = result == (int64_t)0;
	std::uint8_t nVal = result < 0;

	SRFlag::set(cpsr, SRFlag::n, nVal);
	SRFlag::set(cpsr, SRFlag::z, zVal);
  }
}

void CPU::ArmSingleDataSwap_P(ParamList params) {
  spdlog::get("std")->trace("SDS");
  const std::uint32_t Rm = params[0], Rd = params[1], Rn = params[2],
                      B = params[3];
  ArmSingleDataSwap(B, Rn, Rd, Rm);
}

void CPU::ArmSingleDataSwap(std::uint32_t B,
                            std::uint32_t Rn,
                            std::uint32_t Rd,
                            std::uint32_t Rm) {
  if (Rm == 15 || Rn == 15 || Rd == 15) {
	spdlog::get("std")->error("Invalid Arm SDS Using PC");
	return;
  }

  clock->Tick(1);
  if (B) {
	auto addr = registers.get((Register)Rn);
	auto memVal = memory->Read(AccessSize::Byte, addr, Sequentiality::NSEQ);
	memory->Write(AccessSize::Byte, addr, registers.get((Register)Rm),
	              Sequentiality::SEQ);
	registers.get((Register)Rd) = memVal;
  } else {
	auto addr = registers.get((Register)Rn);
	auto memVal = memory->Read(AccessSize::Word, addr, Sequentiality::NSEQ);
	// TODO: Maybe these writes need to be word aligned?
	memory->Write(AccessSize::Word, addr, registers.get((Register)Rm),
	              Sequentiality::SEQ);
	registers.get((Register)Rd) = memVal;
  }
}

void CPU::ArmBranchAndExchange_P(ParamList params) {
  const std::uint32_t Rn = params[0];
  ArmBranchAndExchange(Rn);
}

void CPU::ArmBranchAndExchange(std::uint32_t Rn) {
  auto val = registers.get((Register)Rn);

  spdlog::get("std")->trace("BRANCHEXCHANGE {:X}", val);
  SRFlag::set(registers.get(CPSR), SRFlag::thumb, val & NBIT_MASK(1));
  registers.get(Register::R15) = val;
  PipelineFlush();
}

void CPU::ArmHalfwordDTRegOffset_P(ParamList params) {
  spdlog::get("std")->trace("HDT Reg");
  std::uint32_t Rm = params[0], H = params[1], S = params[2], Rd = params[3],
                Rn = params[4], L = params[5], W = params[6], U = params[7],
                P = params[8];
  ArmHalfwordDTRegOffset(P, U, W, L, Rn, Rd, S, H, Rm);
}

void CPU::ArmHalfwordDTRegOffset(std::uint32_t P,
                                 std::uint32_t U,
                                 std::uint32_t W,
                                 std::uint32_t L,
                                 std::uint32_t Rn,
                                 std::uint32_t Rd,
                                 std::uint32_t S,
                                 std::uint32_t H,
                                 std::uint32_t Rm) {
  auto Offset = registers.get((Register)Rm);
  spdlog::get("std")->debug("HDT Reg Offset {:X}", Offset);
  ArmHalfwordDT(P, U, W, L, Rn, Rd, S, H, Offset);
}

void CPU::ArmHalfwordDTImmOffset_P(ParamList params) {
  spdlog::get("std")->trace("HDT Imm");
  const std::uint32_t OffsetLo = params[0], H = params[1], S = params[2],
                      OffsetHi = params[3], Rd = params[4], Rn = params[5],
                      L = params[6], W = params[7], U = params[8],
                      P = params[9];
  ArmHalfwordDTImmOffset(P, U, W, L, Rn, Rd, OffsetHi, S, H, OffsetLo);
}

void CPU::ArmHalfwordDTImmOffset(std::uint32_t P,
                                 std::uint32_t U,
                                 std::uint32_t W,
                                 std::uint32_t L,
                                 std::uint32_t Rn,
                                 std::uint32_t Rd,
                                 std::uint32_t OffsetHi,
                                 std::uint32_t S,
                                 std::uint32_t H,
                                 std::uint32_t OffsetLo) {
  spdlog::get("std")->trace(
      "HDT Imm P{} U{} W{} L{} Rn{} Rd{} OffsetHi{} S{} H{} OffsetLo{}", P, U,
      W, L, Rn, Rd, OffsetHi, S, H, OffsetLo);
  auto Offset = OffsetLo + (OffsetHi << 4);
  ArmHalfwordDT(P, U, W, L, Rn, Rd, S, H, Offset);
}

void CPU::ArmHalfwordDT(std::uint32_t P,
                        std::uint32_t U,
                        std::uint32_t W,
                        std::uint32_t L,
                        std::uint32_t Rn,
                        std::uint32_t Rd,
                        std::uint32_t S,
                        std::uint32_t H,
                        std::uint32_t Offset) {
  auto base = registers.get((Register)Rn);
  auto baseOffset = base;

  if (U) {
	baseOffset += Offset;
  } else {
	baseOffset -= Offset;
  }

  auto memAddr = base;
  spdlog::get("std")->debug("HDT memAddr {:X} baseOffset {:X}", memAddr,
                            baseOffset);
  if (P) {
	memAddr = baseOffset;
  }

  if (W || !P) {
	registers.get((Register)Rn) = baseOffset;
  }

  auto& destReg = registers.get((Register)Rd);

  if (L)  // LD
  {
	clock->Tick(1);
	if (H)  // HalfWord
	{
	  // TODO: Addr needs to be on half boundary
	  destReg = memory->Read(AccessSize::Half, memAddr, Sequentiality::NSEQ);
	  if (S)  // Signed
	  {
		if (destReg >> 15) {
		  destReg |= NBIT_MASK(16) << 16;
		}
	  }
	} else  // Byte
	{
	  if (S) {
		destReg = memory->Read(AccessSize::Byte, memAddr, Sequentiality::NSEQ);
		if (destReg >> 7) {
		  destReg |= NBIT_MASK(24) << 8;
		}
	  } else {
		spdlog::get("std")->error(
		    "Not allowed to do unsigned byte transfer using HDT");
	  }
	}
  } else  // STR
  {
	if (H)  // HalfWord
	{
	  spdlog::get("std")->debug("HDT Store memAddr {:X} value {:X}", memAddr,
	                            destReg);
	  memory->Write(AccessSize::Half, memAddr, destReg, Sequentiality::NSEQ);
	}
  }
}

void CPU::ArmSingleDataTransfer_P(ParamList params) {
  spdlog::get("std")->trace("SDT");
  std::uint32_t Offset = params[0], Rd = params[1], Rn = params[2],
                L = params[3], W = params[4], B = params[5], U = params[6],
                P = params[7], I = params[8];
  ArmSingleDataTransfer(I, P, U, B, W, L, Rn, Rd, Offset);
}

void CPU::ArmSingleDataTransfer(std::uint32_t I,
                                std::uint32_t P,
                                std::uint32_t U,
                                std::uint32_t B,
                                std::uint32_t W,
                                std::uint32_t L,
                                std::uint32_t Rn,
                                std::uint32_t Rd,
                                std::uint32_t Offset) {
  spdlog::get("std")->trace("SDT I{} P{} U{} B{} W{} L{} Rn{} Rd{} Offset{}", I,
                            P, U, B, W, L, Rn, Rd, Offset);
  if (I) {
	auto carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
	auto Rm = registers.get((Register)(BIT_RANGE(Offset, 0, 3)));
	auto shiftType = BIT_RANGE(Offset, 5, 6);
	auto shiftAmount = BIT_RANGE(Offset, 7, 11);
	Shift(Rm, shiftAmount, shiftType, carry, false);
	Offset = Rm;
  }

  auto base = registers.get((Register)Rn);
  auto baseOffset = base;

  if (U) {
	baseOffset += Offset;
  } else {
	baseOffset -= Offset;
  }

  auto memAddr = base;
  if (P) {
	memAddr = baseOffset;
  }

  if (W || !P) {
	registers.get((Register)Rn) = baseOffset;
  }

  auto& destReg = registers.get((Register)Rd);

  if (L) {
	clock->Tick(1);
	if (B) {
	  destReg = memory->Read(AccessSize::Byte, memAddr, Sequentiality::NSEQ);
	} else {
	  auto wordBoundaryOffset = memAddr % 4;
	  auto value = memory->Read(AccessSize::Word, memAddr - wordBoundaryOffset,
	                            Sequentiality::NSEQ);
	  if (wordBoundaryOffset) {
		std::uint8_t emptyCarry = 0;
		const std::uint32_t ROR = 0b11;
		spdlog::get("std")->trace("Word Boundary Offset {} for value {:X}",
		                          wordBoundaryOffset, value);
		Shift(value, wordBoundaryOffset * 8, ROR, emptyCarry, false);
	  }
	  spdlog::get("std")->trace("R{:X} <- {:X}", Rd, value);
	  destReg = value;
	  if (Rd == 15) {
		PipelineFlush();
	  }
	}
  } else {
	if (B) {
	  memory->Write(AccessSize::Byte, memAddr, destReg, Sequentiality::NSEQ);
	} else {
	  // TODO: Check word boundary addr
	  memory->Write(AccessSize::Word, memAddr, destReg, Sequentiality::NSEQ);
	}
  }
}

void CPU::ArmUndefined_P(ParamList) {
  ArmUndefined();
}

void CPU::ArmUndefined() {
  spdlog::get("std")->trace("UND");

  registers.switchMode(SRFlag::ModeBits::UND);

  registers.get(R14) = registers.get(R15) - 4;
  SRFlag::set(registers.get(CPSR), SRFlag::irqDisable, 1);
  registers.get(R15) = 0x04;

  PipelineFlush();
}

void CPU::ArmBlockDataTransfer_P(ParamList params) {
  spdlog::get("std")->trace("BDT");
  std::uint32_t RegList = params[0], Rn = params[1], L = params[2],
                W = params[3], S = params[4], U = params[5], P = params[6];
  ArmBlockDataTransfer(P, U, S, W, L, Rn, RegList);
}

void CPU::ArmBlockDataTransfer(std::uint32_t P,
                               std::uint32_t U,
                               std::uint32_t S,
                               std::uint32_t W,
                               std::uint32_t L,
                               std::uint32_t Rn,
                               std::uint32_t RegList) {
  auto base = registers.get((Register)Rn);
  spdlog::get("std")->trace(
      "BDT P{:X} U{:X} S{:X} W{:X} L{:X} Rn{:X} RegList{:X}", P, U, S, W, L, Rn,
      RegList);
  std::vector<Register> toSave;
  for (uint8_t i = 0; i < 16; i++) {
	if ((RegList >> i) & NBIT_MASK(1)) {
	  toSave.emplace_back((Register)i);
	}
  }
  bool transferPC = BIT_RANGE(RegList, 15, 15);

  SRFlag::ModeBits mode = SRFlag::ModeBits::USR;
  // R15 not in list and S bit set (User bank transfer)
  if (S && (!transferPC || !L)) {
	mode = (SRFlag::ModeBits)SRFlag::get(registers.get(CPSR), SRFlag::modeBits);
	registers.switchMode(SRFlag::ModeBits::USR);
  }

  auto addr = base;
  if (U)  // up
  {
	if (P)  // pre
	{
	  addr += 4;
	}
  } else  // down
  {
	addr -= 4 * toSave.size();
	if (!P)  // post
	{
	  addr += 4;
	}
  }
  bool stopWriteback = false;

  auto writebackVal = base - 4 * toSave.size();
  if (U) {
	writebackVal = base + 4 * toSave.size();
  }

  auto saved = 0;
  for (auto reg : toSave) {
	auto accessType = Sequentiality::SEQ;
	if (saved == 0) {
	  accessType = Sequentiality::NSEQ;
	}
	if (L) {
	  if (reg == (Register)Rn) {
		stopWriteback = true;
	  }
	  registers.get(reg) = memory->Read(AccessSize::Word, addr, accessType);
	} else {
	  if (reg == (Register)Rn) {
		if (saved == 0) {
		  memory->Write(AccessSize::Word, addr, base, accessType);
		} else {
		  memory->Write(AccessSize::Word, addr, writebackVal, accessType);
		}
	  } else {
		memory->Write(AccessSize::Word, addr, registers.get(reg), accessType);
	  }
	}
	addr += 4;
	saved++;
  }

  if (S && L && transferPC) {
	auto previousSPSR = registers.get(SPSR);
	registers.switchMode(
	    (SRFlag::ModeBits)SRFlag::get(registers.get(SPSR), SRFlag::modeBits));
	registers.get(CPSR) = previousSPSR;
  }

  if (W && !stopWriteback) {
	registers.get((Register)Rn) = writebackVal;
  }

  // Change back to previous mode
  if (S && (!transferPC || !L)) {
	registers.switchMode(mode);
  }

  if (transferPC && L) {
	PipelineFlush();
  }
}

void CPU::ArmBranch_P(ParamList params) {
  spdlog::get("std")->trace("BRANCH");
  std::uint32_t Offset = params[0], L = params[1];
  ArmBranch(L, Offset);
}

void CPU::ArmBranch(std::uint32_t L, std::uint32_t Offset) {
  Offset <<= 2;
  spdlog::get("std")->trace("BRANCH L{:X} Offset:{:X}", L, Offset);
  std::int32_t signedOffset = (Offset & NBIT_MASK(25));
  if (Offset >> 25) {
	signedOffset -= (1 << 25);
  }

  if (L) {
	registers.get(Register::R14) =
	    (registers.get(Register::R15) - 4) & ~NBIT_MASK(2);
  }

  registers.get(Register::R15) += signedOffset;
  PipelineFlush();
}

void CPU::ArmSWI_P(ParamList) {
  ArmSWI();
}

void CPU::ArmSWI() {
  spdlog::get("std")->trace("SWI");

  registers.switchMode(SRFlag::ModeBits::SVC);
  registers.get(R14) = registers.get(R15) - 4;

  SRFlag::set(registers.get(CPSR), SRFlag::irqDisable, 1);
  registers.get(R15) = 0x8;

  PipelineFlush();
}

}  // namespace ARM7TDMI
