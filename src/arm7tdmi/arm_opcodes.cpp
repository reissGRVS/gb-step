#include "cpu.hpp"
#include "spdlog/spdlog.h"

#define BIT_MASK(N) ((1 << N) - 1)

// TODO: TIMINGS
namespace ARM7TDMI {
ParamList CPU::ParseParams(OpCode opcode, ParamSegments paramSegs) {
  ParamList params;
  for (auto it = paramSegs.rbegin(); it != paramSegs.rend(); ++it) {
	auto param =
	    (opcode >> it->second) & BIT_MASK((it->first - it->second + 1));
	params.push_back(param);
  }
  return params;
}

void CPU::Shift(std::uint32_t& value,
                std::uint32_t amount,
                const std::uint32_t& shiftType,
                std::uint8_t& carryOut) {
  switch (shiftType) {
	case 0b00:  // LSL
	{
	  value <<= (amount - 1);
	  carryOut = value >> 31;
	  value <<= 1;
	  break;
	}
	case 0b01:  // LSR
	case 0b10:  // ASR
	{
	  auto neg = amount >> 31;
	  if (amount == 0) {
		amount = 32;
	  }
	  value >>= (amount - 1);
	  carryOut = amount & 1;
	  value >>= 1;

	  if (shiftType == 0b10 && neg) {
		value |= (BIT_MASK(amount) << (32 - amount));
	  }
	  break;
	}
	case 0b11:  // RR
	{
	  // RRX
	  if (amount == 0) {
		auto carryCopy = carryOut;
		carryOut = value & 1;
		value >>= 1;
		if (carryCopy) {
		  value |= 1 << 31;
		}
	  }
	  // ROR
	  else {
		auto topHalf = value << (32 - amount);

		value >>= (amount - 1);
		carryOut = amount & 1;
		value >>= 1;
		value |= topHalf;
	  }
	  break;
	}
	default:  // Should not be possible
	{
	  spdlog::error("CPU::Shift invalid parameters");
	  break;
	}
  }
}

std::function<void()> CPU::ArmOperation(OpCode opcode) {
  if (!registers.conditionCheck((Condition)(opcode >> 28))) {
	return []() {
	  spdlog::debug("Condition failed");
	  return;
	};
  }

  switch (opcode >> 26 & BIT_MASK(2)) {
	case 0b00: {
	  if (opcode & (1 << 25)) {
		return std::bind(&CPU::ArmDataProcessing, this,
		                 ParseParams(opcode, DataProcessingSegments));
	  } else if ((opcode & 0xFFFFFF0) == 0x12FFF10) {
		return std::bind(&CPU::ArmBranchAndExchange, this,
		                 ParseParams(opcode, BranchAndExchangeSegments));
	  } else if ((opcode & 0x18000F0) == 0x0000090) {
		return std::bind(&CPU::ArmMultiply, this,
		                 ParseParams(opcode, MultiplySegments));
	  } else if ((opcode & 0x18000F0) == 0x0800090) {
		return std::bind(&CPU::ArmMultiplyLong, this,
		                 ParseParams(opcode, MultiplyLongSegments));
	  } else if ((opcode & 0x1B00FF0) == 0x1000090) {
		return std::bind(&CPU::ArmSingleDataSwap, this,
		                 ParseParams(opcode, SingleDataSwapSegments));
	  } else if ((opcode & 0xF0) == 0xB0 || (opcode & 0xF0) == 0xD0 ||
	             (opcode & 0xF0) == 0xF0) {
		if (opcode & (1 << 22)) {
		  return std::bind(&CPU::ArmHalfwordDTImmOffset, this,
		                   ParseParams(opcode, HalfwordDTImmOffsetSegments));
		} else {
		  return std::bind(&CPU::ArmHalfwordDTRegOffset, this,
		                   ParseParams(opcode, HalfwordDTRegOffsetSegments));
		}
	  } else {
		return std::bind(&CPU::ArmDataProcessing, this,
		                 ParseParams(opcode, DataProcessingSegments));
	  }
	}
	case 0b01:  // SDT and Undef
	{
	  std::uint32_t undefMask = (0b11 << 25) | (0b1 << 4);

	  if ((opcode & undefMask) == undefMask) {
		return std::bind(&CPU::ArmUndefined, this, ParamList());
	  } else {
		return std::bind(&CPU::ArmSingleDataTransfer, this,
		                 ParseParams(opcode, SingleDataTransferSegments));
	  }
	}
	case 0b10:  // BDT and Branch
	{
	  if (opcode & (1 << 25)) {
		return std::bind(&CPU::ArmBranch, this,
		                 ParseParams(opcode, BranchSegments));
	  } else {
		return std::bind(&CPU::ArmBlockDataTransfer, this,
		                 ParseParams(opcode, BlockDataTransferSegments));
	  }
	}
	case 0b11:  // CoProc and SWI
	{
	  const auto swiMask = 0xF000000;
	  if ((swiMask & opcode) == swiMask) {
		return std::bind(&CPU::ArmSWI, this, ParamList());
	  }
	}
	default: {
	  spdlog::error("This should be impossible");
	  exit(-1);
	}
  }
}

void CPU::ArmDataProcessing(ParamList params) {
  std::uint32_t Op2 = params[0], Rd = params[1], Rn = params[2], S = params[3],
                OpCode = params[4], I = params[5];
  spdlog::debug("DP I:{:X} OpCode:{:X} S:{:X} Rn:{:X} Rd:{:X} Op2:{:X}", I,
                OpCode, S, I, Rn, Rd, Op2);

  std::uint32_t Op1Val = registers.get((Register)Rn);
  auto& dest = registers.get((Register)Rd);
  auto carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
  std::uint32_t Op2Val = 0;

  // TODO: Deal with R15 operand edge case
  if (!I) {
	auto& Rm = registers.get((Register)(Op2 & BIT_MASK(4)));
	auto shiftType = Op2 >> 5 & BIT_MASK(2);
	auto shiftAmount = Op2 >> 7;

	if (Op2 >> 4 & BIT_MASK(1))  // Shift amount from register
	{
	  if (Op2 >> 7 & BIT_MASK(1)) {
		spdlog::error("This instruction should have been UNDEF or MUL");
	  }
	  shiftAmount = registers.get((Register)(shiftAmount >> 1)) & BIT_MASK(8);
	}

	Shift(Rm, shiftAmount, shiftType, carry);
	Op2Val = Rm;
  } else {
	auto Imm = Op2 & BIT_MASK(8);
	auto rotate = (Op2 >> 8) * 2;
	const std::uint32_t ROR = 0b11;
	Shift(Imm, rotate, ROR, carry);
	Op2Val = Imm;
  }

  auto SetFlags = [this](const std::uint32_t& S, const std::uint32_t& result,
                         const std::uint8_t& carry) {
	if (S) {
	  auto& cpsr = registers.get(CPSR);
	  std::uint8_t zVal = result == 0;
	  std::uint8_t nVal = result >> 31;
	  SRFlag::set(cpsr, SRFlag::n, nVal);
	  SRFlag::set(cpsr, SRFlag::z, zVal);
	  SRFlag::set(cpsr, SRFlag::c, carry);
	}
  };

  auto& cpsr = registers.get(CPSR);

  switch ((DPOps)OpCode) {
	case DPOps::AND: {
	  dest = Op1Val & Op2Val;
	  spdlog::debug("	AND {} {} {}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::EOR: {
	  dest = Op1Val ^ Op2Val;
	  spdlog::debug("	EOR {} {} {}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::SUB: {
	  std::uint64_t result = Op1Val - Op2Val;
	  dest = (uint32_t)result;
	  spdlog::debug("	SUB {} {} {}", Op1Val, Op2Val, dest);
	  carry = Op1Val >= Op2Val;
	  auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}
	case DPOps::RSB: {
	  std::uint64_t result = Op2Val - Op1Val;
	  dest = (uint32_t)result;
	  spdlog::debug("	RSB {} {} {}", Op1Val, Op2Val, dest);
	  carry = Op2Val >= Op1Val;
	  auto overflow = ((Op1Val ^ Op2Val) & ~(Op1Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::ADD: {
	  std::uint64_t result = Op1Val + Op2Val;
	  dest = (uint32_t)result;
	  spdlog::debug("	ADD {} {} {}", Op1Val, Op2Val, dest);
	  carry = result >> 32;
	  auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	// TODO: Revisit all flag logic
	case DPOps::ADC: {
	  std::uint64_t result = Op1Val + Op2Val + carry;
	  dest = (uint32_t)result;
	  spdlog::debug("	ADC {} {} {} {}", Op1Val, Op2Val, carry, dest);
	  auto overflow = ((~(Op1Val ^ Op2Val) & ((Op1Val + Op2Val) ^ Op2Val)) ^
	                   (~((Op1Val + Op2Val) ^ carry) & (dest ^ carry))) >>
	                  31;
	  carry = result >> 32;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::SBC: {
	  std::uint32_t Op3Val = carry ^ 1;
	  dest = Op1Val - Op2Val - Op3Val;
	  spdlog::debug("	SBC {} {} {} {}", Op1Val, Op2Val, Op3Val, dest);
	  carry = (Op1Val >= Op2Val) && ((Op1Val - Op2Val) >= (Op3Val));
	  auto overflow = (((Op1Val ^ Op2Val) & ~((Op1Val - Op2Val) ^ Op2Val)) ^
	                   ((Op1Val - Op2Val) & ~dest)) >>
	                  31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::RSC: {
	  std::uint32_t Op3Val = carry ^ 1;
	  dest = Op2Val - Op1Val - Op3Val;
	  spdlog::debug("	RSC {} {} {} {}", Op1Val, Op2Val, Op3Val, dest);
	  carry = (Op2Val >= Op1Val) && ((Op2Val - Op1Val) >= (Op3Val));
	  auto overflow = (((Op2Val ^ Op1Val) & ~((Op2Val - Op1Val) ^ Op1Val)) ^
	                   ((Op2Val - Op1Val) & ~dest)) >>
	                  31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  break;
	}

	case DPOps::TST: {
	  auto result = Op1Val & Op2Val;
	  spdlog::debug("	CMP {} {} {}", Op1Val, Op2Val, result);
	  SetFlags(1, result, carry);
	  S = 0;  // To not trigger SetFlags at bottom
	  break;
	}

	case DPOps::TEQ: {
	  auto result = Op1Val ^ Op2Val;
	  spdlog::debug("	TEQ {} {} {}", Op1Val, Op2Val, result);
	  SetFlags(1, result, carry);
	  S = 0;
	  break;
	}

	case DPOps::CMP: {
	  auto result = Op1Val - Op2Val;
	  spdlog::debug("	CMP {} {} {}", Op1Val, Op2Val, result);
	  carry = Op1Val >= Op2Val;
	  SetFlags(1, result, carry);
	  auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  S = 0;
	  break;
	}

	case DPOps::CMN: {
	  std::uint64_t resultBig = Op1Val + Op2Val;
	  std::uint32_t result = (uint32_t)resultBig;
	  spdlog::debug("	CMN {} {} {}", Op1Val, Op2Val, result);
	  carry = resultBig >> 32;
	  SetFlags(1, result, carry);
	  auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ dest)) >> 31;
	  SRFlag::set(cpsr, SRFlag::v, overflow);
	  S = 0;
	  break;
	}

	case DPOps::ORR: {
	  dest = Op1Val | Op2Val;
	  spdlog::debug("	ORR {} {} {}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::MOV: {
	  dest = Op2Val;
	  spdlog::debug("	MOV {}", dest);
	  break;
	}

	case DPOps::BIC: {
	  dest = Op2Val & ~Op2Val;
	  spdlog::debug("	BIC {} {} {}", Op1Val, Op2Val, dest);
	  break;
	}

	case DPOps::MVN: {
	  dest = ~Op2Val;
	  spdlog::debug("	MVN {}", dest);
	  break;
	}

	default:
	  break;
  }

  SetFlags(S, dest, carry);
}

void CPU::ArmMultiply(ParamList params) {
  spdlog::debug("MUL");
  std::uint32_t Rm = params[0], Rs = params[1], Rn = params[2], Rd = params[3],
                S = params[4], A = params[5];

  auto& dest = registers.get((Register)Rd);

  if (Rd == Rm) {
	spdlog::error("Invalid Arm Mult Rd == Rm");
	return;
  }

  if (Rm == 15 || Rs == 15 || Rn == 15 || Rd == 15) {
	spdlog::error("Invalid Arm Mult Using PC");
	return;
  }

  std::int32_t op1 = registers.get((Register)Rm);
  std::int32_t op2 = registers.get((Register)Rs);
  std::int32_t op3 = registers.get((Register)Rn);

  if (A) {
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

void CPU::ArmMultiplyLong(ParamList params) {
  spdlog::debug("MULLONG");
  std::uint32_t Rm = params[0], Rs = params[1], RdLo = params[2],
                RdHi = params[3], S = params[4], A = params[5], U = params[5];

  if (RdLo == Rm || RdHi == Rm || RdLo == RdHi) {
	spdlog::error("Invalid Arm Mult Long Rd == Rm");
	return;
  }

  if (Rm == 15 || Rs == 15 || RdHi == 15 || RdLo == 15) {
	spdlog::error("Invalid Arm Mult Long Using PC");
	return;
  }

  std::int64_t result = 0;
  std::uint32_t op1 = registers.get((Register)Rm);
  std::uint32_t op2 = registers.get((Register)Rs);
  std::uint32_t op3 = registers.get((Register)RdLo);
  std::uint32_t op4 = registers.get((Register)RdHi);

  if (A) {
	result = op4;
	result <<= 32;
	result += op3;
  }

  if (U) {
	result += op1 * op2;
  } else {
	result += (int32_t)op1 * (int32_t)op2;
  }

  registers.get((Register)RdLo) = (uint32_t)result;
  registers.get((Register)RdHi) = result >> 32;

  if (S) {
	auto& cpsr = registers.get(CPSR);
	std::uint8_t zVal = result == 0;
	std::uint8_t nVal = result >> 63;
	SRFlag::set(cpsr, SRFlag::n, nVal);
	SRFlag::set(cpsr, SRFlag::z, zVal);
  }
}

void CPU::ArmSingleDataSwap(ParamList params) {
  spdlog::debug("SDS");
  std::uint32_t Rm = params[0], Rd = params[1], Rn = params[2], B = params[3];

  if (Rm == 15 || Rn == 15 || Rd == 15) {
	spdlog::error("Invalid Arm SDS Using PC");
	return;
  }

  if (B) {
	auto addr = registers.get((Register)Rn);
	auto memVal = memory->Read(Memory::AccessSize::Byte, addr,
	                           Memory::Sequentiality::NSEQ);
	memory->Write(Memory::AccessSize::Byte, addr, registers.get((Register)Rm),
	              Memory::Sequentiality::NSEQ);
	registers.get((Register)Rd) = memVal;
  } else {
	auto addr = registers.get((Register)Rn);
	auto memVal = memory->Read(Memory::AccessSize::Word, addr,
	                           Memory::Sequentiality::NSEQ);
	// TODO: Maybe these writes need to be word aligned?
	memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm),
	              Memory::Sequentiality::NSEQ);
	memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm),
	              Memory::Sequentiality::NSEQ);
	memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm),
	              Memory::Sequentiality::NSEQ);
	memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm),
	              Memory::Sequentiality::NSEQ);
	registers.get((Register)Rd) = memVal;
  }
}

void CPU::ArmBranchAndExchange(ParamList params) {
  std::uint32_t Rn = params[0];
  spdlog::debug("BRANCHEXCHANGE {:X}", Rn);

  if (Rn & BIT_MASK(1)) {
	SRFlag::set(registers.get(CPSR), SRFlag::thumb, 1);
  } else {
	SRFlag::set(registers.get(CPSR), SRFlag::thumb, 0);
  }

  registers.get(Register::R15) = registers.get((Register)Rn);
  PipelineFlush();
}

void CPU::ArmHalfwordDTRegOffset(ParamList params) {
  spdlog::debug("HDT");
  std::uint32_t Rm = params[0], H = params[1], S = params[2], Rd = params[4],
                Rn = params[5], L = params[3], W = params[4], U = params[6],
                P = params[7];

  auto Offset = registers.get((Register)Rm);
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

  auto destReg = registers.get((Register)Rd);

  if (L)  // LD
  {
	if (H)  // HalfWord
	{
	  // TODO: Addr needs to be on half boundary
	  destReg = memory->Read(Memory::AccessSize::Half, memAddr,
	                         Memory::Sequentiality::NSEQ);
	  if (S)  // Signed
	  {
		if (destReg >> 15) {
		  destReg *= -1;
		}
	  }
	} else  // Byte
	{
	  if (S) {
		destReg = memory->Read(Memory::AccessSize::Byte, memAddr,
		                       Memory::Sequentiality::NSEQ);
		if (destReg >> 7) {
		  destReg *= -1;
		}
	  } else {
		spdlog::error("Not allowed to do unsigned byte transfer using HDT");
	  }
	}
  } else  // STR
  {
	if (H)  // HalfWord
	{
	  memory->Write(Memory::AccessSize::Byte, memAddr, destReg,
	                Memory::Sequentiality::NSEQ);
	  memory->Write(Memory::AccessSize::Byte, memAddr + 2, destReg,
	                Memory::Sequentiality::NSEQ);
	}
  }

  if (W || !P) {
	registers.get((Register)Rn) = baseOffset;
  }
}

// TODO: Tidy up data transfers to use common functions, large sections of
// repetition
void CPU::ArmHalfwordDTImmOffset(ParamList params) {
  spdlog::debug("HDT");
  std::uint32_t OffsetLo = params[0], H = params[1], S = params[2],
                OffsetHi = params[3], Rd = params[4], Rn = params[5],
                L = params[3], W = params[4], U = params[6], P = params[7];

  auto Offset = OffsetLo + (OffsetHi << 4);
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

  auto destReg = registers.get((Register)Rd);

  if (L)  // LD
  {
	if (H)  // HalfWord
	{
	  // TODO: Addr needs to be on half boundary
	  destReg = memory->Read(Memory::AccessSize::Half, memAddr,
	                         Memory::Sequentiality::NSEQ);
	  if (S)  // Signed
	  {
		if (destReg >> 15) {
		  destReg *= -1;
		}
	  }
	} else  // Byte
	{
	  if (S) {
		destReg = memory->Read(Memory::AccessSize::Byte, memAddr,
		                       Memory::Sequentiality::NSEQ);
		if (destReg >> 7) {
		  destReg *= -1;
		}
	  } else {
		spdlog::error("Not allowed to do unsigned byte transfer using HDT");
	  }
	}
  } else  // STR
  {
	if (H)  // HalfWord
	{
	  memory->Write(Memory::AccessSize::Byte, memAddr, destReg,
	                Memory::Sequentiality::NSEQ);
	  memory->Write(Memory::AccessSize::Byte, memAddr + 2, destReg,
	                Memory::Sequentiality::NSEQ);
	}
  }

  if (W || !P) {
	registers.get((Register)Rn) = baseOffset;
  }
}

void CPU::ArmSingleDataTransfer(ParamList params) {
  spdlog::debug("SDT");
  std::uint32_t Offset = params[0], Rd = params[1], Rn = params[2],
                L = params[3], W = params[4], B = params[5], U = params[6],
                P = params[7], I = params[8];

  if (I) {
	auto carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
	auto& Rm = registers.get((Register)(Offset & BIT_MASK(4)));
	auto shiftType = Offset >> 5 & BIT_MASK(2);
	auto shiftAmount = Offset >> 7;
	Shift(Rm, shiftAmount, shiftType, carry);
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

  auto destReg = registers.get((Register)Rd);

  if (L) {
	if (B) {
	  destReg = memory->Read(Memory::AccessSize::Byte, memAddr,
	                         Memory::Sequentiality::NSEQ);
	} else {
	  // TODO: Check word boundary addr LD behaviour, seems complicated pg 49 or
	  // 55 of pdf
	  destReg = memory->Read(Memory::AccessSize::Word, memAddr,
	                         Memory::Sequentiality::NSEQ);
	}
  } else {
	if (B) {
	  memory->Write(Memory::AccessSize::Byte, memAddr, destReg,
	                Memory::Sequentiality::NSEQ);
	  memory->Write(Memory::AccessSize::Byte, memAddr + 1, destReg,
	                Memory::Sequentiality::NSEQ);
	  memory->Write(Memory::AccessSize::Byte, memAddr + 2, destReg,
	                Memory::Sequentiality::NSEQ);
	  memory->Write(Memory::AccessSize::Byte, memAddr + 3, destReg,
	                Memory::Sequentiality::NSEQ);
	} else {
	  // TODO: Check word boundary addr
	  memory->Write(Memory::AccessSize::Word, memAddr, destReg,
	                Memory::Sequentiality::NSEQ);
	}
  }

  if (W || !P) {
	registers.get((Register)Rn) = baseOffset;
  }
}

void CPU::ArmUndefined(ParamList) {
  spdlog::debug("UND");
  auto pc = registers.get(R15) - 4;
  auto v = SRFlag::get(registers.get(CPSR), SRFlag::v);

  registers.switchMode(SRFlag::ModeBits::UND);

  registers.get(R14) = pc;
  SRFlag::set(registers.get(SPSR), SRFlag::v, v);
  SRFlag::set(registers.get(CPSR), SRFlag::irqDisable, 1);
  registers.get(R15) = 0x04;

  PipelineFlush();
}

void CPU::ArmBlockDataTransfer(ParamList params) {
  // TODO: Use of S bit, Use of R15 as base
  spdlog::debug("BDT");
  std::uint32_t RegList = params[0], Rn = params[1], L = params[2],
                W = params[3], S = params[4], U = params[5], P = params[6];

  auto base = registers.get((Register)Rn);
  std::vector<Register> toSave;
  for (uint8_t i = 0; i < 16; i++) {
	if ((RegList >> i) & BIT_MASK(1)) {
	  toSave.emplace_back((Register)i);
	}
  }
  bool transferPC = RegList >> 15 & BIT_MASK(1);

  SRFlag::ModeBits mode;
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
	if (L) {
	  if (reg == (Register)Rn) {
		stopWriteback = true;
	  }
	  registers.get(reg) = memory->Read(Memory::AccessSize::Word, addr,
	                                    Memory::Sequentiality::NSEQ);
	} else {
	  if (reg == (Register)Rn) {
		if (saved == 0) {
		  memory->Write(Memory::AccessSize::Word, addr, base,
		                Memory::Sequentiality::NSEQ);
		} else {
		  memory->Write(Memory::AccessSize::Word, addr, writebackVal,
		                Memory::Sequentiality::NSEQ);
		}
	  } else {
		memory->Write(Memory::AccessSize::Word, addr, registers.get(reg),
		              Memory::Sequentiality::NSEQ);
	  }
	}
	addr += 4;
	saved++;
  }

  if (S && L && transferPC) {
	registers.switchMode(
	    (SRFlag::ModeBits)SRFlag::get(registers.get(SPSR), SRFlag::modeBits));
  }

  if (W && !stopWriteback) {
	registers.get((Register)Rn) = writebackVal;
  }

  // Change back to previous mode
  if (S && (!transferPC || !L)) {
	registers.switchMode(mode);
  }
}

void CPU::ArmBranch(ParamList params) {
  spdlog::debug("BRANCH");
  std::uint32_t Offset = params[0], L = params[1];
  Offset <<= 2;
  std::int32_t signedOffset = (Offset & BIT_MASK(25));
  if (Offset >> 25) {
	signedOffset *= -1;
  }

  if (L) {
	// TODO: Adjust for prefetch
	registers.get(Register::R14) = registers.get(Register::R15) & ~BIT_MASK(2);
  }

  registers.get(Register::R15) += signedOffset;
  PipelineFlush();
}

void CPU::ArmSWI(ParamList) {
  spdlog::debug("SWI");
  auto pc = registers.get(R15) - 4;
  auto v = SRFlag::get(registers.get(CPSR), SRFlag::v);

  registers.switchMode(SRFlag::ModeBits::SVC);

  registers.get(R14) = pc;
  SRFlag::set(registers.get(SPSR), SRFlag::v, v);
  SRFlag::set(registers.get(CPSR), SRFlag::irqDisable, 1);
  registers.get(R15) = 0x08;

  PipelineFlush();
}
}  // namespace ARM7TDMI