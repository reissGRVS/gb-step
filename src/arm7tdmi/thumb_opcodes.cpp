#include "arm7tdmi/cpu.hpp"
#include "platform/logging.hpp"
#include "utils.hpp"

namespace ARM7TDMI {

const ParamSegments MoveShiftedRegSegments
	//  Op        Offset5  Rs      Rd
	= { { 12, 11 }, { 10, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments AddSubtractSegments
	//  I         Op      Rn/Off3 Rs      Rd
	= { { 10, 10 }, { 9, 9 }, { 8, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments MoveCompAddSubImmSegments
	//  Op        Rd      Offset8
	= { { 12, 11 }, { 10, 8 }, { 7, 0 } };

const ParamSegments ALUOpsSegments
	//  Op      Rs      Rd
	= { { 9, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments HiRegOpsSegments
	//  Op      H1      H2      Rs      Rd
	= { { 9, 8 }, { 7, 7 }, { 6, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments PCRelativeLoadSegments
	//  Rd      Word8
	= { { 10, 8 }, { 7, 0 } };

const ParamSegments LSRegOffSegments
	//  L         B         Ro      Rb      Rd
	= { { 11, 11 }, { 10, 10 }, { 8, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments LSSignExtSegments
	//  H         S         Ro      Rb      Rd
	= { { 11, 11 }, { 10, 10 }, { 8, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments LSImmOffSegments
	//  B         L         Off5     Rb      Rd
	= { { 12, 12 }, { 11, 11 }, { 10, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments LSHalfSegments
	//  L         Off5     Rb      Rd
	= { { 11, 11 }, { 10, 6 }, { 5, 3 }, { 2, 0 } };

const ParamSegments SPRelativeLSSegments
	//  L       Rd      Word8
	= { { 11, 11 }, { 10, 8 }, { 7, 0 } };

const ParamSegments LoadAddressSegments
	//  SP       Rd      Word8
	= { { 11, 11 }, { 10, 8 }, { 7, 0 } };

const ParamSegments OffsetSPSegments
	//  S      SWord7
	= { { 7, 7 }, { 6, 0 } };

const ParamSegments PushPopRegSegments
	//  L         R      RList
	= { { 11, 11 }, { 8, 8 }, { 7, 0 } };

const ParamSegments MultipleLSSegments
	//  L         Rb     RList
	= { { 11, 11 }, { 10, 8 }, { 7, 0 } };

const ParamSegments CondBranchSegments
	//  Cond     Soffset8
	= { { 11, 8 }, { 7, 0 } };

const ParamSegments SWISegments
	//  Value8
	= { { 7, 0 } };

const ParamSegments UncondBranchSegments
	//  Offset11
	= { { 10, 0 } };

const ParamSegments LongBranchLinkSegments
	//  H       Offset
	= { { 11, 11 }, { 10, 0 } };

std::function<void()> CPU::ThumbOperation(OpCode opcode)
{
	switch (BIT_RANGE(opcode, 13, 15)) {
	case 0b000: {
		if ((opcode & 0x1800) == 0x1800) {
			ParseParams(opcode, AddSubtractSegments);
			return ThumbAddSubtract_P();
		} else {
			ParseParams(opcode, MoveShiftedRegSegments);
			return ThumbMoveShiftedReg_P();
		}
	}
	case 0b001: {
		ParseParams(opcode, MoveCompAddSubImmSegments);
		return ThumbMoveCompAddSubImm_P();
	}
	case 0b010: {
		switch (BIT_RANGE(opcode, 10, 12)) {
		case 0b000: {

			ParseParams(opcode, ALUOpsSegments);
			return ThumbALUOps_P();
		}
		case 0b001: {

			ParseParams(opcode, HiRegOpsSegments);
			return ThumbHiRegOps_P();
		}
		case 0b010:
		case 0b011: {

			ParseParams(opcode, PCRelativeLoadSegments);
			return ThumbPCRelativeLoad_P();
		}
		default: {
			if (BIT_RANGE(opcode, 9, 9)) {

				ParseParams(opcode, LSSignExtSegments);
				return ThumbLSSignExt_P();
			} else {
				ParseParams(opcode, LSRegOffSegments);
				return ThumbLSRegOff_P();
			}
		}
		}
	}
	case 0b011: {

		ParseParams(opcode, LSImmOffSegments);
		return ThumbLSImmOff_P();
	}
	case 0b100: {
		if (BIT_RANGE(opcode, 12, 12)) {

			ParseParams(opcode, SPRelativeLSSegments);
			return ThumbSPRelativeLS_P();
		} else {

			ParseParams(opcode, LSHalfSegments);
			return ThumbLSHalf_P();
		}
	}
	case 0b101: {
		if (BIT_RANGE(opcode, 12, 12)) {
			if (BIT_RANGE(opcode, 8, 11)) {

				ParseParams(opcode, PushPopRegSegments);
				return ThumbPushPopReg_P();
			} else {

				ParseParams(opcode, OffsetSPSegments);
				return ThumbOffsetSP_P();
			}
		} else {

			ParseParams(opcode, LoadAddressSegments);
			return ThumbLoadAddress_P();
		}
	}
	case 0b110: {
		if (BIT_RANGE(opcode, 12, 12)) {
			if (BIT_RANGE(opcode, 8, 11) == NBIT_MASK(4)) {
				ParseParams(opcode, SWISegments);
				return std::bind(&CPU::ThumbSWI, this);
			} else {
				ParseParams(opcode, CondBranchSegments);
				return ThumbCondBranch_P();
			}
		} else {
			ParseParams(opcode, MultipleLSSegments);
			return ThumbMultipleLS_P();
		}
	}
	case 0b111: {
		if (BIT_RANGE(opcode, 12, 12)) {
			ParseParams(opcode, LongBranchLinkSegments);
			return ThumbLongBranchLink_P();
		} else {
			ParseParams(opcode, UncondBranchSegments);
			return ThumbUncondBranch_P();
		}
	}

	default:
		LOG_ERROR("Invalid Thumb Instruction: This should never happen")
		exit(-1);
		break;
	}
}

std::function<void()> CPU::ThumbMoveShiftedReg_P()
{
	U16 Rd = params[0], Rs = params[1], Offset5 = params[2], Op = params[3];

	if (Op == 0b11) {
		LOG_ERROR("Invalid op for Thumb MSR")
		exit(-1);
	}

	auto Op2 = Rs + (Offset5 << 7) + (Op << 5);
	return std::bind(&CPU::ArmDataProcessing, this, 0, DPOps::MOV, 1, 0, Rd, Op2);
}

std::function<void()> CPU::ThumbAddSubtract_P()
{
	U16 Rd = params[0], Rs = params[1], Rn = params[2], Op = params[3],
		I = params[4];

	auto dpOp = static_cast<U32>(Op ? DPOps::SUB : DPOps::ADD);
	return std::bind(&CPU::ArmDataProcessing, this, I, dpOp, 1, Rs, Rd, Rn);
}

std::function<void()> CPU::ThumbMoveCompAddSubImm_P()
{
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
		LOG_ERROR("ThumbMoveCompAddSubImm invalid Op")
		exit(-1);
	}

	return std::bind(&CPU::ArmDataProcessing, this, 1, dpOp, S, Rd, Rd, Offset8);
}

std::function<void()> CPU::ThumbALUOps_P()
{
	U16 Rd = params[0], Rs = params[1], Op = params[2];

	switch (Op) {
	case 0b0010: {
		// LSL
		U16 Op2 = (Rs << 8) + (0b001 << 4) + Rd;
		return std::bind(&CPU::ArmDataProcessing, this, 0, DPOps::MOV, 1, Rd, Rd, Op2);
		break;
	}
	case 0b0011: {
		// LSR
		U16 Op2 = (Rs << 8) + (0b011 << 4) + Rd;
		return std::bind(&CPU::ArmDataProcessing, this, 0, DPOps::MOV, 1, Rd, Rd, Op2);
		break;
	}
	case 0b0100: {
		// ASR
		U16 Op2 = (Rs << 8) + (0b101 << 4) + Rd;
		return std::bind(&CPU::ArmDataProcessing, this, 0, DPOps::MOV, 1, Rd, Rd, Op2);
		break;
	}
	case 0b0111: {
		// ROR
		U16 Op2 = (Rs << 8) + (0b111 << 4) + Rd;
		return std::bind(&CPU::ArmDataProcessing, this, 0, DPOps::MOV, 1, Rd, Rd, Op2);
		break;
	}
	case 0b1001: {
		// NEG
		return std::bind(&CPU::ArmDataProcessing, this, 1, DPOps::RSB, 1, Rs, Rd, 0);
		break;
	}
	case 0b1101: {
		// MUL
		return std::bind(&CPU::ArmMultiply, this, 0, 1, Rd, 0, Rd, Rs);
		break;
	}
	default: {
		// Directly mapped ops
		return std::bind(&CPU::ArmDataProcessing, this, 0, Op, 1, Rd, Rd, Rs);
		break;
	}
	}
}

std::function<void()> CPU::ThumbHiRegOps_P()
{
	U16 Rd = params[0], Rs = params[1], H2 = params[2], H1 = params[3],
		Op = params[4];
	// TODO: Detect unhandled cases for this Op and complain

	auto Hd = Rd + (H1 << 3);
	auto Hs = Rs + (H2 << 3);

	if (Op == 0b11) {
		return std::bind(&CPU::ArmBranchAndExchange, this, Hs);
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
			LOG_ERROR("ThumbMoveCompAddSubImm invalid Op {}", Op)
			exit(-1);
		}
		return std::bind(&CPU::ArmDataProcessing, this, 0, dpOp, S, Hd, Hd, Hs);
	}
}

std::function<void()> CPU::ThumbPCRelativeLoad_P()
{
	U16 Word8 = params[0], Rd = params[1];
	return std::bind(&CPU::ThumbPCRelativeLoad, this, Word8, Rd);
}

void CPU::ThumbPCRelativeLoad(U16 Word8, U16 Rd)
{
	auto offsetFix = (registers.get(R15)) % 4;
	return ArmSingleDataTransfer(0, 1, 1, 0, 0, 1, Register::R15, Rd, (Word8 << 2) - offsetFix);
}

// Load/Store
std::function<void()> CPU::ThumbLSRegOff_P()
{
	U16 Rd = params[0], Rb = params[1], Ro = params[2], B = params[3],
		L = params[4];

	return std::bind(&CPU::ArmSingleDataTransfer, this, 1, 1, 1, B, 0, L, Rb, Rd, Ro);
}

std::function<void()> CPU::ThumbLSSignExt_P()
{
	U16 Rd = params[0], Rb = params[1], Ro = params[2], S = params[3],
		H = params[4];

	if (S | H) {
		// Loads
		return std::bind(&CPU::ArmHalfwordDTRegOffset, this, 1, 1, 0, 1, Rb, Rd, S, H, Ro);
	} else {
		// Store
		return std::bind(&CPU::ArmHalfwordDTRegOffset, this, 1, 1, 0, 0, Rb, Rd, 0, 1, Ro);
	}
}

std::function<void()> CPU::ThumbLSImmOff_P()
{

	U16 Rd = params[0], Rb = params[1], Offset5 = params[2], L = params[3],
		B = params[4];

	U16 Offset = Offset5;
	if (!B) {
		Offset = Offset5 << 2;
	}
	return std::bind(&CPU::ArmSingleDataTransfer, this, 0, 1, 1, B, 0, L, Rb, Rd, Offset);
}

std::function<void()> CPU::ThumbLSHalf_P()
{
	U16 Rd = params[0], Rb = params[1], Offset5 = params[2], L = params[3];

	auto OffsetHi = Offset5 >> 3;
	auto OffsetLo = (Offset5 << 1) & NBIT_MASK(4);
	return std::bind(&CPU::ArmHalfwordDTImmOffset, this, 1, 1, 0, L, Rb, Rd, OffsetHi, 0, 1, OffsetLo);
}

std::function<void()> CPU::ThumbSPRelativeLS_P()
{
	U16 Word8 = params[0], Rd = params[1], L = params[2];

	return std::bind(&CPU::ArmSingleDataTransfer, this, 0, 1, 1, 0, 0, L, Register::R13, Rd, Word8 << 2);
}

std::function<void()> CPU::ThumbLoadAddress_P()
{
	U16 Word8 = params[0], Rd = params[1], SP = params[2];

	U32 Rn = SP ? Register::R13 : Register::R15;

	const auto ROR30 = (0xF << 8);
	return std::bind(&CPU::DataProcessing, this, 1, DPOps::ADD, 0, Rn, Rd, ROR30 + Word8, true);
}

std::function<void()> CPU::ThumbOffsetSP_P()
{
	U16 SWord7 = params[0], S = params[1];

	auto dpOp = static_cast<U32>(S ? DPOps::SUB : DPOps::ADD);
	const auto ROR30 = (0xF << 8);
	return std::bind(&CPU::ArmDataProcessing, this, 1, dpOp, 1, Register::R13, Register::R13, ROR30 + SWord7);
}

std::function<void()> CPU::ThumbPushPopReg_P()
{
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
	return std::bind(&CPU::ArmBlockDataTransfer, this, 1 ^ L, 0 ^ L, 0, 1, L, Register::R13, RList);
}

std::function<void()> CPU::ThumbMultipleLS_P()
{

	U16 RList = params[0], Rb = params[1], L = params[2];
	return std::bind(&CPU::ArmBlockDataTransfer, this, 0, 1, 0, 1, L, Rb, RList);
}

std::function<void()> CPU::ThumbCondBranch_P()
{
	U16 SOffset8 = params[0], Cond = params[1];
	return std::bind(&CPU::ThumbCondBranch, this, SOffset8, Cond);
}

void CPU::ThumbCondBranch(U16 SOffset8, U16 Cond)
{
	if (!registers.ConditionCheck((Condition)(Cond))) {

		return;
	}

	S16 offset = (SOffset8 & NBIT_MASK(7)) << 1;
	if (SOffset8 >> 7) {
		offset -= 1 << 8;
	}
	auto& pc = registers.get(Register::R15);

	pc += offset;
	PipelineFlush();
}

void CPU::ThumbSWI()
{

	registers.SwitchMode(ModeBits::SVC);

	registers.get(R14) = registers.get(R15) - 2;
	registers.CPSR.irqDisable = true;
	registers.CPSR.thumb = false;
	registers.get(R15) = 0x8;

	PipelineFlush();
}

std::function<void()> CPU::ThumbUncondBranch_P()
{
	U16 Offset11 = params[0];
	return std::bind(&CPU::ThumbUncondBranch, this, Offset11);
}

void CPU::ThumbUncondBranch(U16 Offset11)
{
	S16 offset = (Offset11 & NBIT_MASK(10)) << 1;
	if (Offset11 >> 10) {
		offset -= 1 << 11;
	}
	registers.get(Register::R15) += offset;
	PipelineFlush();
}

std::function<void()> CPU::ThumbLongBranchLink_P()
{
	U16 Offset = params[0], H = params[1];
	return std::bind(&CPU::ThumbLongBranchLink, this, Offset, H);
}

void CPU::ThumbLongBranchLink(U16 Offset, U16 H)
{
	auto& lr = registers.get(Register::R14);
	auto& pc = registers.get(Register::R15);
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