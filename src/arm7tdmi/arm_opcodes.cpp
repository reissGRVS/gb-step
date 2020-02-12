#include "arm7tdmi/cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <unistd.h>

namespace ARM7TDMI {


#define EXTRA_PC_INC (registers.CPSR.thumb ? 2 : 4)

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

void CPU::Shift(U32 &value, U32 amount, const U32 &shiftType, bool &carryOut,
                bool regProvidedAmount) {
  switch (shiftType) {
	case 0b00: // LSL
	{
		if (amount > 0 && amount < 32) {
			value <<= (amount - 1);
			carryOut = value >> 31;
			value <<= 1;
		}
		else if (amount == 32)
		{
			carryOut = value & 1;
			value = 0;
		}
		else if (regProvidedAmount && amount > 32u)
		{
			carryOut = 0;
			value = 0;
		}
		break;
	}
  case 0b01: // LSR
  case 0b10: // ASR
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
    else if (amount == 0) {
      if (regProvidedAmount) {

        return;
      } else {
        amount = 32;
      }
    }

    value >>= (amount - 1);
    carryOut = value & 1;
    value >>= 1;
    if (neg && shiftType == 0b10) {
      U32 mask = (NBIT_MASK(amount) << (32 - amount));
      value |= mask;
    }
    break;
  }
  case 0b11: // RR
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
  default: // Should not be possible
  {

    break;
  }
  }
}

Op CPU::ArmOperation(OpCode opcode) {
	
  switch (BIT_RANGE(opcode, 26, 27)) {
  case 0b00: {
    if (opcode & (1 << 25)) {
	  ParseParams(opcode, DataProcessingSegments);
      return ArmDataProcessing_P();
    } else if ((opcode & 0xFFFFFF0) == 0x12FFF10) {
	  ParseParams(opcode, BranchAndExchangeSegments);
      return ArmBranchAndExchange_P();
    } else if ((opcode & 0x18000F0) == 0x0000090) {
	  ParseParams(opcode, MultiplySegments);
      return ArmMultiply_P();
    } else if ((opcode & 0x18000F0) == 0x0800090) {
	  ParseParams(opcode, MultiplyLongSegments);
      return ArmMultiplyLong_P();
    } else if ((opcode & 0x1B00FF0) == 0x1000090) {
      ParseParams(opcode, SingleDataSwapSegments);
      return ArmSingleDataSwap_P();
    } else if ((opcode & 0xF0) == 0xB0 || (opcode & 0xF0) == 0xD0 ||
               (opcode & 0xF0) == 0xF0) {
      if (opcode & (1 << 22)) {
        ParseParams(opcode, HalfwordDTImmOffsetSegments);
        return ArmHalfwordDTImmOffset_P();
      } else {
		ParseParams(opcode, HalfwordDTRegOffsetSegments);
        return ArmHalfwordDTRegOffset_P();             
      }
    } else {
      ParseParams(opcode, DataProcessingSegments);
      return ArmDataProcessing_P();
    }
  }
  case 0b01: // SDT and Undef
  {
    U32 undefMask = (0b11 << 25) | (0b1 << 4);

    if ((opcode & undefMask) == undefMask) {
      return ArmUndefined_P();
    } else {
      ParseParams(opcode, SingleDataTransferSegments);
      return ArmSingleDataTransfer_P();
    }
  }
  case 0b10: // BDT and Branch
  {
    if (opcode & (1 << 25)) {
	  ParseParams(opcode, BranchSegments);
      return ArmBranch_P();                
    } else {
      ParseParams(opcode, BlockDataTransferSegments);
      return ArmBlockDataTransfer_P();
    }
  }
  case 0b11: // CoProc and SWI
  {
    const auto swiMask = 0xF000000;
    if ((swiMask & opcode) == swiMask) {
      return ArmSWI_P();
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

void CPU::ArmMRS(bool Ps, U8 Rd) {
  auto &dest = registers.get((Register)Rd);
  if (Ps) {
    dest = registers.GetSPSR().ToU32();
  } else {
    dest = registers.CPSR.ToU32();
  }
}

void CPU::ArmMSR(bool I, bool Pd, bool flagsOnly, U16 source) {
  auto value = registers.get((Register)(source & NBIT_MASK(4)));
  if (I) {
      value = (source & NBIT_MASK(8));
      auto rotate = (source >> 8) * 2;
      const U32 ROR = 0b11;
      // Maybe this should do RRX as well?
      bool carry = 0;
      if (rotate) {
        Shift(value, rotate, ROR, carry, false);
      }
    }

  if (!flagsOnly) {
    if (Pd) {
      registers.GetSPSR().FromU32(value);
    } else {
      registers.CPSR.FromU32(value);
      registers.SwitchMode(registers.CPSR.modeBits);
    }
  } else {
    
    value >>= 28;

    if (Pd) {
	  registers.GetSPSR().FlagsFromU4(value);
    } else {
	  registers.CPSR.FlagsFromU4(value);
    }
  }
}

Op CPU::ArmDataProcessing_P() {
  const U32 Op2 = params[0], Rd = params[1], Rn = params[2], S = params[3],
            OpCode = params[4], I = params[5];
  
  return std::bind(&CPU::ArmDataProcessing, this, I, OpCode, S, Rn, Rd, Op2);
}

void CPU::ArmDataProcessing(U32 I, U32 OpCode, U32 S, U32 Rn, U32 Rd, U32 Op2) {
	DataProcessing(I, OpCode, S, Rn, Rd, Op2, false);
}

void CPU::DataProcessing(U32 I, U32 OpCode, U32 S, U32 Rn, U32 Rd, U32 Op2, bool Adr) {
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

  U32 Op1Val = registers.get((Register)Rn);

  // If using PC, bit 1 is cleared, only for ADR instructions if in thumb
  if (Rn == 15 && (Adr || !registers.CPSR.thumb)) {
    BIT_CLEAR(Op1Val, 1);
  }

  
  auto carry = registers.CPSR.c;
  U32 Op2Val = 0;
  const DataProcOperand2& op2Params = operand2Parameters[Op2];
  if (!I) {
    auto Rm = registers.get(op2Params.rm);

    if (op2Params.fromReg) // Shift amount from register
    {
      // If also using R15 to specify extra shifts
      if (op2Params.rm == R15) {
        Rm += EXTRA_PC_INC;
      }
	  if (Rn == 15) {
        Op1Val += EXTRA_PC_INC;
      }

      clock->Tick(1);
      auto shiftAmount = registers.get(op2Params.shiftRegister) & NBIT_MASK(8);
      Shift(Rm, shiftAmount, op2Params.shiftType, carry, true);
    } else {
      Shift(Rm, op2Params.shiftAmount, op2Params.shiftType, carry, false);
    }
    Op2Val = Rm;
  } else {
    auto Imm = op2Params.imm;
    if (op2Params.rotate) {
      const U32 ROR = 0b11;
      Shift(Imm, op2Params.rotate, ROR, carry, false);
    }

    Op2Val = Imm;
  }

  if (Rd == 15 && S) {
    auto previousSPSR = registers.GetSPSR();
    registers.SwitchMode(previousSPSR.modeBits);
    registers.CPSR = previousSPSR;
    S = 0;
  }

  auto &dest = registers.get((Register)Rd);

  

  auto SetFlags = [this](const U32 &S, const U32 &result, const U8 &carry) {
    if (S) {
      registers.CPSR.n = BIT_RANGE(result, 31, 31);
      registers.CPSR.z = (result == 0);
      registers.CPSR.c = carry;
    }
  };
  switch (static_cast<DPOps>(OpCode)) {
  case DPOps::AND: {
    dest = Op1Val & Op2Val;
    break;
  }

  case DPOps::EOR: {
    dest = Op1Val ^ Op2Val;
    break;
  }

  case DPOps::SUB: {
    auto result = (std::uint64_t)Op1Val - Op2Val;
    dest = (uint32_t)result;
    carry = Op1Val >= Op2Val;
    auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ dest)) >> 31;
	registers.CPSR.v = overflow;
    break;
  }
  case DPOps::RSB: {
    auto result = (std::uint64_t)Op2Val - Op1Val;
    dest = (uint32_t)result;
    carry = Op2Val >= Op1Val;
    auto overflow = ((Op1Val ^ Op2Val) & ~(Op1Val ^ dest)) >> 31;
    registers.CPSR.v = overflow;
    break;
  }

  case DPOps::ADD: {
    auto result = (std::uint64_t)Op1Val + Op2Val;
    dest = (uint32_t)result;
    carry = result >> 32;
    auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ dest)) >> 31;
    registers.CPSR.v = overflow;
    break;
  }

  case DPOps::ADC: {
    carry = registers.CPSR.c;
    auto result = (std::uint64_t)Op1Val + Op2Val + carry;
    dest = (uint32_t)result;
    auto overflow = ((~(Op1Val ^ Op2Val) & ((Op1Val + Op2Val) ^ Op2Val)) ^
                     (~((Op1Val + Op2Val) ^ carry) & (dest ^ carry))) >>
                    31;
    carry = result >> 32;
    registers.CPSR.v = overflow;
    break;
  }

  case DPOps::SBC: {
    carry = registers.CPSR.c;
    U32 Op3Val = carry ^ 1;
    dest = Op1Val - Op2Val - Op3Val;
    carry = (Op1Val >= Op2Val) && ((Op1Val - Op2Val) >= (Op3Val));
    auto overflow = (((Op1Val ^ Op2Val) & ~((Op1Val - Op2Val) ^ Op2Val)) ^
                     ((Op1Val - Op2Val) & ~dest)) >>
                    31;
    registers.CPSR.v = overflow;
    break;
  }

  case DPOps::RSC: {
    carry = registers.CPSR.c;
    U32 Op3Val = carry ^ 1;
    dest = Op2Val - Op1Val - Op3Val;
    carry = (Op2Val >= Op1Val) && ((Op2Val - Op1Val) >= (Op3Val));
    auto overflow = (((Op2Val ^ Op1Val) & ~((Op2Val - Op1Val) ^ Op1Val)) ^
                     ((Op2Val - Op1Val) & ~dest)) >>
                    31;
    registers.CPSR.v = overflow;
    break;
  }

  case DPOps::TST: {
    auto result = Op1Val & Op2Val;
    SetFlags(1, result, carry);
    S = 0; // To not trigger SetFlags at bottom
    break;
  }

  case DPOps::TEQ: {
    auto result = Op1Val ^ Op2Val;
    SetFlags(1, result, carry);
    S = 0;
    break;
  }

  case DPOps::CMP: {
    auto result = (std::uint64_t)Op1Val - Op2Val;
    carry = Op1Val >= Op2Val;
    SetFlags(1, result, carry);
    auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ ((U32)result))) >> 31;
    registers.CPSR.v = overflow;
    S = 0;
    break;
  }

  case DPOps::CMN: {
    auto resultBig = (std::uint64_t)Op1Val + Op2Val;
    U32 result = (uint32_t)resultBig;
    carry = resultBig >> 32;
    SetFlags(1, result, carry);
    auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ result)) >> 31;
    registers.CPSR.v = overflow;
    S = 0;
    break;
  }

  case DPOps::ORR: {
    dest = Op1Val | Op2Val;
    break;
  }

  case DPOps::MOV: {
    dest = Op2Val;
    break;
  }

  case DPOps::BIC: {
    dest = Op1Val & ~Op2Val;

    break;
  }

  case DPOps::MVN: {
    dest = ~Op2Val;

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

void CPU::ICyclesMultiply(const U32 &mulop) {
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

Op CPU::ArmMultiply_P() {

  const U32 Rm = params[0], Rs = params[1], Rn = params[2], Rd = params[3],
            S = params[4], A = params[5];
  return std::bind(&CPU::ArmMultiply, this, A, S, Rd, Rn, Rs, Rm);
}

void CPU::ArmMultiply(U32 A, U32 S, U32 Rd, U32 Rn, U32 Rs, U32 Rm) {
  auto &dest = registers.get((Register)Rd);

  if (Rd == Rm) {

    return;
  }

  if (Rm == 15 || Rs == 15 || Rn == 15 || Rd == 15) {

    return;
  }

  S32 op1 = registers.get((Register)Rm);
  S32 op2 = registers.get((Register)Rs);
  ICyclesMultiply(op2);
  S32 op3 = registers.get((Register)Rn);

  if (A) {
    clock->Tick(1);
    dest = op1 * op2 + op3;
  } else {
    dest = op1 * op2;
  }

  if (S) {
    registers.CPSR.n = (dest >> 31);
    registers.CPSR.z = (dest == 0);
  }
}

Op CPU::ArmMultiplyLong_P() {

  const U32 Rm = params[0], Rs = params[1], RdLo = params[2], RdHi = params[3],
            S = params[4], A = params[5], U = params[6];
  return std::bind(&CPU::ArmMultiplyLong, this, U, A, S, RdHi, RdLo, Rs, Rm);
}

void CPU::ArmMultiplyLong(U32 U, U32 A, U32 S, U32 RdHi, U32 RdLo, U32 Rs,
                          U32 Rm) {
  if (RdLo == Rm || RdHi == Rm || RdLo == RdHi) {

    return;
  }

  if (Rm == 15 || Rs == 15 || RdHi == 15 || RdLo == 15) {

    return;
  }

  clock->Tick(1);
  std::int64_t result = 0;
  U32 op1 = registers.get((Register)Rm);
  U32 op2 = registers.get((Register)Rs);
  ICyclesMultiply(op2);
  U32 op3 = registers.get((Register)RdLo);
  U32 op4 = registers.get((Register)RdHi);

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
	registers.CPSR.n = (result < 0);
	registers.CPSR.z = (result == (int64_t)0);
  }
}

Op CPU::ArmSingleDataSwap_P() {

  const U32 Rm = params[0], Rd = params[1], Rn = params[2], B = params[3];
  return std::bind(&CPU::ArmSingleDataSwap, this, B, Rn, Rd, Rm);
}

void CPU::ArmSingleDataSwap(U32 B, U32 Rn, U32 Rd, U32 Rm) {
  if (Rm == 15 || Rn == 15 || Rd == 15) {

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
	auto wordBoundaryOffset = addr % 4;  

    auto memVal = memory->Read(AccessSize::Word, addr - wordBoundaryOffset, Sequentiality::NSEQ);
    if (wordBoundaryOffset) {
        bool emptyCarry = 0;
        const U32 ROR = 0b11;
        Shift(memVal, wordBoundaryOffset * 8, ROR, emptyCarry, false);
    }

    memory->Write(AccessSize::Word, addr - wordBoundaryOffset, registers.get((Register)Rm),
                  Sequentiality::SEQ);
    registers.get((Register)Rd) = memVal;
  }
}

Op CPU::ArmBranchAndExchange_P() {
  const U32 Rn = params[0];
  return std::bind(&CPU::ArmBranchAndExchange, this, Rn);
}

void CPU::ArmBranchAndExchange(U32 Rn) {
  auto val = registers.get((Register)Rn);

  registers.CPSR.thumb = val & NBIT_MASK(1);
  registers.get(Register::R15) = val;
  PipelineFlush();
}

Op CPU::ArmHalfwordDTRegOffset_P() {

  U32 Rm = params[0], H = params[1], S = params[2], Rd = params[3],
      Rn = params[4], L = params[5], W = params[6], U = params[7],
      P = params[8];
  return std::bind(&CPU::ArmHalfwordDTRegOffset, this, P, U, W, L, Rn, Rd, S, H, Rm);
}

void CPU::ArmHalfwordDTRegOffset(U32 P, U32 U, U32 W, U32 L, U32 Rn, U32 Rd,
                                 U32 S, U32 H, U32 Rm) {
  auto Offset = registers.get((Register)Rm);

  ArmHalfwordDT(P, U, W, L, Rn, Rd, S, H, Offset);
}

Op CPU::ArmHalfwordDTImmOffset_P() {
  const U32 OffsetLo = params[0], H = params[1], S = params[2],
            OffsetHi = params[3], Rd = params[4], Rn = params[5], L = params[6],
            W = params[7], U = params[8], P = params[9];
  return std::bind(&CPU::ArmHalfwordDTImmOffset, this, P, U, W, L, Rn, Rd, OffsetHi, S, H, OffsetLo);
}

void CPU::ArmHalfwordDTImmOffset(U32 P, U32 U, U32 W, U32 L, U32 Rn, U32 Rd,
                                 U32 OffsetHi, U32 S, U32 H, U32 OffsetLo) {
  auto Offset = OffsetLo + (OffsetHi << 4);
  ArmHalfwordDT(P, U, W, L, Rn, Rd, S, H, Offset);
}

void CPU::ArmHalfwordDT(U32 P, U32 U, U32 W, U32 L, U32 Rn, U32 Rd, U32 S,
                        U32 H, U32 Offset) {

  auto value = registers.get((Register)Rd);
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

  auto &destReg = registers.get((Register)Rd);

  if (L) // LD
  {
    clock->Tick(1);
    if (H) // HalfWord
    {
      // TODO: Addr needs to be on half boundary

	  auto wordBoundaryOffset = memAddr & 1;
      
      if (wordBoundaryOffset) {
		spdlog::get("std")->warn("half word boundary offset logic potentially wrong");
		auto value = memory->Read(AccessSize::Word, memAddr - wordBoundaryOffset,
                            Sequentiality::NSEQ);
        bool emptyCarry = 0;
        const U32 ROR = 0b11;
        Shift(value, wordBoundaryOffset * 8, ROR, emptyCarry, false);
		destReg = value;
      }
	  else{
      	destReg = memory->Read(AccessSize::Half, memAddr, Sequentiality::NSEQ);
	  }

      if (S) // Signed
      {
		

        if (destReg >> 15) {
          destReg |= NBIT_MASK(16) << 16;
        }
		else if (wordBoundaryOffset && BIT_RANGE(destReg, 7, 7))
		{
			spdlog::get("std")->warn("signed half word offset logic potentially wrong", destReg);
			destReg |= NBIT_MASK(24) << 8;
		}
		
      }
    } else // Byte
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
  } else // STR
  {
    if (H) // HalfWord
    {
	  if (Rd==15) value += EXTRA_PC_INC;
	  auto memOffset = memAddr & 1;
      memory->Write(AccessSize::Half, memAddr - memOffset, value, Sequentiality::NSEQ);
    }
  }
}

Op CPU::ArmSingleDataTransfer_P() {

  U32 Offset = params[0], Rd = params[1], Rn = params[2], L = params[3],
      W = params[4], B = params[5], U = params[6], P = params[7], I = params[8];
  return std::bind(&CPU::ArmSingleDataTransfer, this, I, P, U, B, W, L, Rn, Rd, Offset);
}

void CPU::ArmSingleDataTransfer(U32 I, U32 P, U32 U, U32 B, U32 W, U32 L,
                                U32 Rn, U32 Rd, U32 Offset) {
  if (I) {
    auto carry = registers.CPSR.c;
    auto Rm = registers.get((Register)(BIT_RANGE(Offset, 0, 3)));
    auto shiftType = BIT_RANGE(Offset, 5, 6);
    auto shiftAmount = BIT_RANGE(Offset, 7, 11);
    Shift(Rm, shiftAmount, shiftType, carry, false);
    Offset = Rm;
  }


  auto value = registers.get((Register)Rd);
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

  

  if (L) {
	auto &destReg = registers.get((Register)Rd);
    clock->Tick(1);
    if (B) {
      destReg = memory->Read(AccessSize::Byte, memAddr, Sequentiality::NSEQ);
    } else {
      auto wordBoundaryOffset = memAddr % 4;
      auto value = memory->Read(AccessSize::Word, memAddr - wordBoundaryOffset,
                                Sequentiality::NSEQ);
      if (wordBoundaryOffset) {
        bool emptyCarry = 0;
        const U32 ROR = 0b11;
        Shift(value, wordBoundaryOffset * 8, ROR, emptyCarry, false);
      }

      destReg = value;
      if (Rd == 15) {
        PipelineFlush();
      }
    }
  } else {
	if (Rd==15) value += EXTRA_PC_INC;
    if (B) {
      memory->Write(AccessSize::Byte, memAddr, value, Sequentiality::NSEQ);
    } else {
	  auto wordBoundaryOffset = memAddr % 4;  
      memory->Write(AccessSize::Word, memAddr - wordBoundaryOffset, value, Sequentiality::NSEQ);
    }
  }
}

Op CPU::ArmUndefined_P() { return std::bind(&CPU::ArmUndefined, this); }

void CPU::ArmUndefined() {

  registers.SwitchMode(ModeBits::UND);

  registers.get(R14) = registers.get(R15) - 4;
  registers.CPSR.irqDisable = 1;
  registers.get(R15) = 0x04;

  PipelineFlush();
}

Op CPU::ArmBlockDataTransfer_P() {

  U32 RegList = params[0], Rn = params[1], L = params[2], W = params[3],
      S = params[4], U = params[5], P = params[6];
  return std::bind(&CPU::ArmBlockDataTransfer, this, P, U, S, W, L, Rn, RegList);
}

void CPU::ArmBlockDataTransfer(U32 P, U32 U, U32 S, U32 W, U32 L, U32 Rn,
                               U32 RegList) {
  auto base = registers.get((Register)Rn);
  std::vector<Register> toSave;
  for (uint8_t i = 0; i < 16; i++) {
    if ((RegList >> i) & NBIT_MASK(1)) {
      toSave.emplace_back((Register)i);
    }
  }
  bool transferPC = BIT_RANGE(RegList, 15, 15);

  auto mode = ModeBits::USR;
  // R15 not in list and S bit set (User bank transfer)
  if (S && (!transferPC || !L)) {
    mode = registers.CPSR.modeBits;
    registers.SwitchMode(ModeBits::USR);
  }

  auto addr = base;
  if (U) // up
  {
    if (P) // pre
    {
      addr += 4;
    }
  } else // down
  {
    addr -= 4 * toSave.size();
    if (!P) // post
    {
      addr += 4;
    }
  }
  bool stopWriteback = false;

  auto wordCount = toSave.size();
  if (wordCount == 0) 
  {
  	wordCount = 16;
	if (L)
	{
		registers.get(R15) = memory->Read(AccessSize::Word, addr, NSEQ);
	}
	else
	{
		memory->Write(AccessSize::Word, addr, registers.get(R15)+EXTRA_PC_INC, NSEQ);
	}
	
  }

  auto writebackVal = base - 4 * wordCount;
  if (U) {
    writebackVal = base + 4 * wordCount;
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
		auto value = registers.get(reg);
		if (reg == 15) value += EXTRA_PC_INC;
        memory->Write(AccessSize::Word, addr, value, accessType);
      }
    }
    addr += 4;
    saved++;
  }

  if (S && L && transferPC) {
    auto previousSPSR = registers.GetSPSR();
    registers.SwitchMode(previousSPSR.modeBits);
    registers.CPSR = previousSPSR;
  }

  if (W && !stopWriteback) {
    registers.get((Register)Rn) = writebackVal;
  }

  // Change back to previous mode
  if (S && (!transferPC || !L)) {
    registers.SwitchMode(mode);
  }

  if (L && (transferPC || !saved )) {
    PipelineFlush();
  }
}

Op CPU::ArmBranch_P() {

  U32 Offset = params[0], L = params[1];
  return std::bind(&CPU::ArmBranch, this, L, Offset);
}

void CPU::ArmBranch(U32 L, U32 Offset) {
  Offset <<= 2;

  S32 signedOffset = (Offset & NBIT_MASK(25));
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

Op CPU::ArmSWI_P() { return std::bind(&CPU::ArmSWI, this); }

void CPU::ArmSWI() {

  registers.SwitchMode(ModeBits::SVC);
  registers.get(R14) = registers.get(R15) - 4;

  registers.CPSR.irqDisable = 1;
  registers.get(R15) = 0x8;

  PipelineFlush();
}

} // namespace ARM7TDMI
