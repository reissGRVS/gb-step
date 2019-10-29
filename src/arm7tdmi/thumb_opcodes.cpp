#include "cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

// TODO: TIMINGS
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
  switch (opcode >> 13) {
	case 0b000: {
	  if (opcode && 0x1800) {
		// TODO: Op2
	  } else {
		// TODO: Op1
	  }
	  break;
	}
	case 0b001: {
	  // TODO: Op3
	  break;
	}
	case 0b010: {
	  switch (opcode >> 10 & BIT_MASK(3)) {
		case 0b000: {
		  // TODO Op4
		  break;
		}
		case 0b001: {
		  // TODO Op5
		  break;
		}
		case 0b010:
		case 0b011: {
		  // TODO Op6
		  break;
		}
		default: {
		  if (opcode >> 9 & BIT_MASK(1)) {
			// TODO Op8
		  } else {
			// TODO Op7
		  }
		}
	  }
	}
	case 0b011: {
	  // TODO: Op9
	  break;
	}
	case 0b100: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		// TODO: Op11
	  } else {
		// TODO: Op10
	  }
	  break;
	}
	case 0b101: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		if (opcode >> 8 & BIT_MASK(4)) {
		  // TODO: Op14
		} else {
		  // TODO: Op13
		}
	  } else {
		// TODO: Op12
	  }
	  break;
	}
	case 0b110: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		if ((opcode >> 8 & BIT_MASK(4)) == BIT_MASK(4)) {
		  // TODO: Op17
		} else {
		  // TODO: Op16
		}
	  } else {
		// TODO: Op15
	  }
	  break;
	}
	case 0b111: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		// TODO: Op19
	  } else {
		// TODO: Op18
	  }
	  break;
	}

	default:
	  spdlog::error("Invalid Thumb Instruction: This should never happen");
	  break;
  }
}

void CPU::ThumbMoveShiftedReg_P(ParamList params) {
  // TODO:
  std::uint16_t Rd = params[0], Rs = params[1], Offset5 = params[2],
                Op = params[3];
}

void CPU::ThumbAddSubtract_P(ParamList params) {
  std::uint16_t Rd = params[0], Rs = params[1], Rn = params[2], Op = params[3],
                I = params[4];
}

void CPU::ThumbMoveCompAddSubImm_P(ParamList params) {
  std::uint16_t Offset8 = params[0], Rd = params[1], Op = params[2];
}

void CPU::ThumbALUOps_P(ParamList params) {
  std::uint16_t Rd = params[0], Rs = params[1], Op = params[2];
}

void CPU::ThumbHiRegOps_P(ParamList params) {
  std::uint16_t Rd = params[0], Rs = params[1], H2 = params[2], H1 = params[3],
                Op = params[4];
}

void CPU::ThumbPCRelativeLoad_P(ParamList params) {
  std::uint16_t Word8 = params[0], Rd = params[1];
}
// Load/Store
void CPU::ThumbLSRegOff_P(ParamList params) {
  std::uint16_t Rd = params[0], Rb = params[1], Ro = params[2], B = params[3],
                L = params[4];
}

void CPU::ThumbLSSignExt_P(ParamList params) {
  std::uint16_t Rd = params[0], Rb = params[1], Ro = params[2], S = params[3],
                H = params[4];
}

void CPU::ThumbLSImmOff_P(ParamList params) {
  std::uint16_t Rd = params[0], Rb = params[1], Offset5 = params[2],
                L = params[3], B = params[4];
}

void CPU::ThumbLSHalf_P(ParamList params) {
  std::uint16_t Rd = params[0], Rb = params[1], Offset5 = params[2],
                L = params[3];
}

void CPU::ThumbSPRelativeLS_P(ParamList params) {
  std::uint16_t Word8 = params[0], Rd = params[1], L = params[2];
}

void CPU::ThumbLoadAddress_P(ParamList params) {
  std::uint16_t Word8 = params[0], Rd = params[1], SP = params[2];
}

void CPU::ThumbOffsetSP_P(ParamList params) {
  std::uint16_t SWord7 = params[0], S = params[1],
}

void CPU::ThumbPushPopReg_P(ParamList params) {
  std::uint16_t RList = params[0], R = params[1], L = params[2];
}

void CPU::ThumbMultipleLS_P(ParamList params) {
  std::uint16_t RList = params[0], Rb = params[1], L = params[2];
}

void CPU::ThumbCondBranch_P(ParamList params) {
  std::uint16_t SOffset8 = params[0], Cond = params[1];
}

void CPU::ThumbSWI_P(ParamList params) {
  std::uint16_t Value8 = params[0];
}

void CPU::ThumbUncondBranch_P(ParamList params) {
  std::uint16_t Offset11 = params[0];
}

void CPU::ThumbLongBranchLink_P(ParamList params) {
  std::uint16_t Offset = params[0], H = params[1];
}

}  // namespace ARM7TDMI