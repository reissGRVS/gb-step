#include "arm7tdmi/cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace ARM7TDMI {

const ParamSegments MoveShiftedRegSegments
    //  Op        Offset5  Rs      Rd
    = {{12, 11}, {10, 6}, {5, 3}, {2, 0}};

const ParamSegments AddSubtractSegments
    //  I         Op      Rn/Off3 Rs      Rd
    = {{10, 10}, {9, 9}, {8, 6}, {5, 3}, {2, 0}};

const ParamSegments MoveCompAddSubImmSegments
    //  Op        Rd      Offset8
    = {{12, 11}, {10, 8}, {7, 0}};

const ParamSegments ALUOpsSegments
    //  Op      Rs      Rd
    = {{9, 6}, {5, 3}, {2, 0}};

const ParamSegments HiRegOpsSegments
    //  Op      H1      H2      Rs      Rd
    = {{9, 8}, {7, 7}, {6, 6}, {5, 3}, {2, 0}};

const ParamSegments PCRelativeLoadSegments
    //  Rd      Word8
    = {{10, 8}, {7, 0}};

const ParamSegments LSRegOffSegments
    //  L         B         Ro      Rb      Rd
    = {{11, 11}, {10, 10}, {8, 6}, {5, 3}, {2, 0}};

const ParamSegments LSSignExtSegments
    //  H         S         Ro      Rb      Rd
    = {{11, 11}, {10, 10}, {8, 6}, {5, 3}, {2, 0}};

const ParamSegments LSImmOffSegments
    //  B         L         Off5     Rb      Rd
    = {{12, 12}, {11, 11}, {10, 6}, {5, 3}, {2, 0}};

const ParamSegments LSHalfSegments
    //  L         Off5     Rb      Rd
    = {{11, 11}, {10, 6}, {5, 3}, {2, 0}};

const ParamSegments SPRelativeLSSegments
    //  L       Rd      Word8
    = {{11, 11}, {10, 8}, {7, 0}};

const ParamSegments LoadAddressSegments
    //  SP       Rd      Word8
    = {{11, 11}, {10, 8}, {7, 0}};

const ParamSegments OffsetSPSegments
    //  S      SWord7
    = {{7, 7}, {6, 0}};

const ParamSegments PushPopRegSegments
    //  L         R      RList
    = {{11, 11}, {8, 8}, {7, 0}};

const ParamSegments MultipleLSSegments
    //  L         Rb     RList
    = {{11, 11}, {10, 8}, {7, 0}};

const ParamSegments CondBranchSegments
    //  Cond     Soffset8
    = {{11, 8}, {7, 0}};

const ParamSegments SWISegments
    //  Value8
    = {{7, 0}};

const ParamSegments UncondBranchSegments
    //  Offset11
    = {{10, 0}};

const ParamSegments LongBranchLinkSegments
    //  H       Offset
    = {{11, 11}, {10, 0}};

std::function<void()> CPU::ThumbOperation(OpCode opcode) {
  switch (BIT_RANGE(opcode, 13, 15)) {
  case 0b000: {
    if ((opcode & 0x1800) == 0x1800) {
      return std::bind(&CPU::ThumbAddSubtract_P, this,
                       ParseParams(opcode, AddSubtractSegments));
    } else {
      return std::bind(&CPU::ThumbMoveShiftedReg_P, this,
                       ParseParams(opcode, MoveShiftedRegSegments));
    }
    break;
  }
  case 0b001: {
    return std::bind(&CPU::ThumbMoveCompAddSubImm_P, this,
                     ParseParams(opcode, MoveCompAddSubImmSegments));
    break;
  }
  case 0b010: {
    switch (BIT_RANGE(opcode, 10, 12)) {
    case 0b000: {
      return std::bind(&CPU::ThumbALUOps_P, this,
                       ParseParams(opcode, ALUOpsSegments));
      break;
    }
    case 0b001: {
      return std::bind(&CPU::ThumbHiRegOps_P, this,
                       ParseParams(opcode, HiRegOpsSegments));
      break;
    }
    case 0b010:
    case 0b011: {
      return std::bind(&CPU::ThumbPCRelativeLoad_P, this,
                       ParseParams(opcode, PCRelativeLoadSegments));
      break;
    }
    default: {
      if (BIT_RANGE(opcode, 9, 9)) {
        return std::bind(&CPU::ThumbLSSignExt_P, this,
                         ParseParams(opcode, LSSignExtSegments));
      } else {
        return std::bind(&CPU::ThumbLSRegOff_P, this,
                         ParseParams(opcode, LSRegOffSegments));
      }
    }
    }
  }
  case 0b011: {
    return std::bind(&CPU::ThumbLSImmOff_P, this,
                     ParseParams(opcode, LSImmOffSegments));
    break;
  }
  case 0b100: {
    if (BIT_RANGE(opcode, 12, 12)) {
      return std::bind(&CPU::ThumbSPRelativeLS_P, this,
                       ParseParams(opcode, SPRelativeLSSegments));
    } else {
      return std::bind(&CPU::ThumbLSHalf_P, this,
                       ParseParams(opcode, LSHalfSegments));
    }
    break;
  }
  case 0b101: {
    if (BIT_RANGE(opcode, 12, 12)) {
      if (BIT_RANGE(opcode, 8, 11)) {
        return std::bind(&CPU::ThumbPushPopReg_P, this,
                         ParseParams(opcode, PushPopRegSegments));
      } else {
        return std::bind(&CPU::ThumbOffsetSP_P, this,
                         ParseParams(opcode, OffsetSPSegments));
      }
    } else {
      return std::bind(&CPU::ThumbLoadAddress_P, this,
                       ParseParams(opcode, LoadAddressSegments));
    }
    break;
  }
  case 0b110: {
    if (BIT_RANGE(opcode, 12, 12)) {
      if (BIT_RANGE(opcode, 8, 11) == NBIT_MASK(4)) {
        return std::bind(&CPU::ThumbSWI_P, this,
                         ParseParams(opcode, SWISegments));
      } else {
        return std::bind(&CPU::ThumbCondBranch_P, this,
                         ParseParams(opcode, CondBranchSegments));
      }
    } else {
      return std::bind(&CPU::ThumbMultipleLS_P, this,
                       ParseParams(opcode, MultipleLSSegments));
    }
    break;
  }
  case 0b111: {
    if (BIT_RANGE(opcode, 12, 12)) {
      return std::bind(&CPU::ThumbLongBranchLink_P, this,
                       ParseParams(opcode, LongBranchLinkSegments));
    } else {
      return std::bind(&CPU::ThumbUncondBranch_P, this,
                       ParseParams(opcode, UncondBranchSegments));
    }
    break;
  }

  default:
    spdlog::get("std")->error(
        "Invalid Thumb Instruction: This should never happen");
    exit(-1);
    break;
  }
}

void CPU::ThumbMoveShiftedReg_P(ParamList params) {
  U16 Rd = params[0], Rs = params[1], Offset5 = params[2], Op = params[3];

  if (Op == 0b11) {

    exit(-1);
  }

  auto Op2 = Rs + (Offset5 << 7) + (Op << 5);
  ArmDataProcessing(0, DPOps::MOV, 1, 0, Rd, Op2);
}

void CPU::ThumbAddSubtract_P(ParamList params) {
  U16 Rd = params[0], Rs = params[1], Rn = params[2], Op = params[3],
      I = params[4];

  auto dpOp = static_cast<U32>(Op ? DPOps::SUB : DPOps::ADD);
  ArmDataProcessing(I, dpOp, 1, Rs, Rd, Rn);
}

void CPU::ThumbMoveCompAddSubImm_P(ParamList params) {
  U16 Offset8 = params[0], Rd = params[1], Op = params[2];

  U32 dpOp;
  U32 S = 1;
  switch (Op) {
  case 0b00:
    dpOp = DPOps::MOV;
    break;
  case 0b01:
    dpOp = DPOps::CMP;
    break;
  case 0b10:
    dpOp = DPOps::ADD;
    break;
  case 0b11:
    dpOp = DPOps::SUB;
    break;
  default:

    exit(-1);
  }

  ArmDataProcessing(1, dpOp, S, Rd, Rd, Offset8);
}

void CPU::ThumbALUOps_P(ParamList params) {
  U16 Rd = params[0], Rs = params[1], Op = params[2];

  switch (Op) {
  case 0b0010: {
    // LSL
    U16 Op2 = (Rs << 8) + (0b001 << 4) + Rd;
    ArmDataProcessing(0, DPOps::MOV, 1, Rd, Rd, Op2);
    break;
  }
  case 0b0011: {
    // LSR
    U16 Op2 = (Rs << 8) + (0b011 << 4) + Rd;
    ArmDataProcessing(0, DPOps::MOV, 1, Rd, Rd, Op2);
    break;
  }
  case 0b0100: {
    // ASR
    U16 Op2 = (Rs << 8) + (0b101 << 4) + Rd;
    ArmDataProcessing(0, DPOps::MOV, 1, Rd, Rd, Op2);
    break;
  }
  case 0b0111: {
    // ROR
    U16 Op2 = (Rs << 8) + (0b111 << 4) + Rd;
    ArmDataProcessing(0, DPOps::MOV, 1, Rd, Rd, Op2);
    break;
  }
  case 0b1001: {
    // NEG
    ArmDataProcessing(1, DPOps::RSB, 1, Rs, Rd, 0);
    break;
  }
  case 0b1101: {
    // MUL
    ArmMultiply(0, 1, Rd, 0, Rd, Rs);
    break;
  }
  default: {
    // Directly mapped ops
    ArmDataProcessing(0, Op, 1, Rd, Rd, Rs);
    break;
  }
  }
}

void CPU::ThumbHiRegOps_P(ParamList params) {
  U16 Rd = params[0], Rs = params[1], H2 = params[2], H1 = params[3],
      Op = params[4];
  // TODO: Detect unhandled cases for this Op and complain

  auto Hd = Rd + (H1 << 3);
  auto Hs = Rs + (H2 << 3);

  if (Op == 0b11) {
    ArmBranchAndExchange(Hs);
  } else {
    U32 dpOp;
    auto S = 0;
    switch (Op) {
    case 0b00:
      dpOp = DPOps::ADD;
      break;
    case 0b01:
      dpOp = DPOps::CMP;
      S = 1;
      break;
    case 0b10:
      dpOp = DPOps::MOV;
      break;
    default:

      exit(-1);
    }
    ArmDataProcessing(0, dpOp, S, Hd, Hd, Hs);
  }
}

void CPU::ThumbPCRelativeLoad_P(ParamList params) {

  U16 Word8 = params[0], Rd = params[1];
  auto offsetFix = (registers.get(R15)) % 4;
  ArmSingleDataTransfer(0, 1, 1, 0, 0, 1, Register::R15, Rd,
                        (Word8 << 2) - offsetFix);
}
// Load/Store
void CPU::ThumbLSRegOff_P(ParamList params) {
  U16 Rd = params[0], Rb = params[1], Ro = params[2], B = params[3],
      L = params[4];

  spdlog::get("std")->trace(
      "THUMB LS RegOff - Rd {:X}, Rb {:X}, Ro {:X}, B{:X}, L{:X}", Rd, Rb, Ro,
      B, L);
  ArmSingleDataTransfer(1, 1, 1, B, 0, L, Rb, Rd, Ro);
}

void CPU::ThumbLSSignExt_P(ParamList params) {
  U16 Rd = params[0], Rb = params[1], Ro = params[2], S = params[3],
      H = params[4];

  spdlog::get("std")->trace("THUMB LS SignExt Rd {} Rb {} Ro {} S {} H {}", Rd,
                            Rb, Ro, S, H);
  if (S | H) {
    // Loads
    ArmHalfwordDTRegOffset(1, 1, 0, 1, Rb, Rd, S, H, Ro);
  } else {
    // Store
    ArmHalfwordDTRegOffset(1, 1, 0, 0, Rb, Rd, 0, 1, Ro);
  }
}

void CPU::ThumbLSImmOff_P(ParamList params) {

  U16 Rd = params[0], Rb = params[1], Offset5 = params[2], L = params[3],
      B = params[4];

  U16 Offset = Offset5;
  if (!B) {
    Offset = Offset5 << 2;
  }
  ArmSingleDataTransfer(0, 1, 1, B, 0, L, Rb, Rd, Offset);
}

void CPU::ThumbLSHalf_P(ParamList params) {
  U16 Rd = params[0], Rb = params[1], Offset5 = params[2], L = params[3];

  auto OffsetHi = Offset5 >> 3;
  auto OffsetLo = (Offset5 << 1) & NBIT_MASK(4);
  ArmHalfwordDTImmOffset(1, 1, 0, L, Rb, Rd, OffsetHi, 0, 1, OffsetLo);
}

void CPU::ThumbSPRelativeLS_P(ParamList params) {
  U16 Word8 = params[0], Rd = params[1], L = params[2];

  ArmSingleDataTransfer(0, 1, 1, 0, 0, L, Register::R13, Rd, Word8 << 2);
}

void CPU::ThumbLoadAddress_P(ParamList params) {
  U16 Word8 = params[0], Rd = params[1], SP = params[2];

  U32 Rn = SP ? Register::R13 : Register::R15;
  const auto ROR30 = (0xF << 8);
  ArmDataProcessing(1, DPOps::ADD, 0, Rn, Rd, ROR30 + Word8);
}

void CPU::ThumbOffsetSP_P(ParamList params) {
  U16 SWord7 = params[0], S = params[1];

  auto dpOp = static_cast<U32>(S ? DPOps::SUB : DPOps::ADD);
  const auto ROR30 = (0xF << 8);
  ArmDataProcessing(1, dpOp, 1, Register::R13, Register::R13, ROR30 + SWord7);
}

void CPU::ThumbPushPopReg_P(ParamList params) {
  U16 RList = params[0], R = params[1], L = params[2];
  // TODO: Check if direction correct, stack should be full descending

  if (R) {
    if (L) {
      // Add PC to RList
      BIT_SET(RList, 15);
    } else {
      // Add LR to RList
      BIT_SET(RList, 14);
    }
  }
  // STMDB or LDMIA
  ArmBlockDataTransfer(1 ^ L, 0 ^ L, 0, 1, L, Register::R13, RList);
}

void CPU::ThumbMultipleLS_P(ParamList params) {

  U16 RList = params[0], Rb = params[1], L = params[2];
  ArmBlockDataTransfer(0, 1, 0, 1, L, Rb, RList);
}

void CPU::ThumbCondBranch_P(ParamList params) {
  U16 SOffset8 = params[0], Cond = params[1];

  if (!registers.conditionCheck((Condition)(Cond))) {

    return;
  }

  S16 offset = (SOffset8 & NBIT_MASK(7)) << 1;
  if (SOffset8 >> 7) {
    offset -= 1 << 8;
  }
  auto &pc = registers.get(Register::R15);

  pc += offset;
  PipelineFlush();
}

void CPU::ThumbSWI_P(ParamList) {

  registers.switchMode(SRFlag::ModeBits::SVC);

  registers.get(R14) = registers.get(R15) - 2;
  SRFlag::set(registers.get(CPSR), SRFlag::irqDisable, 1);
  SRFlag::set(registers.get(CPSR), SRFlag::thumb, 0);
  registers.get(R15) = 0x8;

  PipelineFlush();
}

void CPU::ThumbUncondBranch_P(ParamList params) {
  U16 Offset11 = params[0];

  S16 offset = (Offset11 & NBIT_MASK(10)) << 1;
  if (Offset11 >> 10) {
    offset -= 1 << 11;
  }
  registers.get(Register::R15) += offset;
  PipelineFlush();
}

#include <unistd.h>
void CPU::ThumbLongBranchLink_P(ParamList params) {
  U16 Offset = params[0], H = params[1];

  auto &lr = registers.get(Register::R14);
  auto &pc = registers.get(Register::R15);
  if (H) {
    lr += (Offset << 1);
    auto temp = pc - 2;
    pc = lr;
    lr = temp | 1;
    PipelineFlush();
  } else {
    S32 signedOffset = ((Offset & NBIT_MASK(10)) << 12);
    if (Offset >> 10) {
      signedOffset -= 1 << 22;
    }

    lr = pc + signedOffset;
  }
}

} // namespace ARM7TDMI