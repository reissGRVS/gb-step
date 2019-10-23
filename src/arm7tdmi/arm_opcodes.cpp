#include "cpu.hpp"

#define BIT_MASK(N) ((1 << N) - 1)


//TODO: TIMINGS
namespace ARM7TDMI
{
ParamList CPU::ParseParams(OpCode opcode, ParamSegments paramSegs)
{
	ParamList params;
	int startPos = 0;
	for (auto it = paramSegs.rbegin(); it != paramSegs.rend(); ++it)
	{
		opcode >>= (it->second - startPos);
		params.push_back(opcode & BIT_MASK(it->second - it->first + 1));
	}
	return params;
}

void CPU::Shift(std::uint32_t &value, std::uint32_t amount, const std::uint32_t &shiftType, std::uint8_t &carryOut)
{
	switch (shiftType)
	{
	case 0x00: //LSL
	{
		value <<= (amount - 1);
		carryOut = value >> 31;
		value <<= 1;
		break;
	}
	case 0x01: //LSR
	case 0x10: //ASR
	{
		auto neg = amount >> 31;
		if (amount == 0)
		{
			amount = 32;
		}
		value >>= (amount - 1);
		carryOut = amount & 1;
		value >>= 1;

		if (shiftType == 0x10 && neg)
		{
			value |= (BIT_MASK(amount) << (32 - amount));
		}
		break;
	}
	case 0x11: //RR
	{
		//RRX
		if (amount == 0)
		{
			auto carryCopy = carryOut;
			carryOut = value & 1;
			value >>= 1;
			if (carryCopy)
			{
				value |= 1 << 31;
			}
		}
		//ROR
		else
		{
			auto topHalf = value << (32 - amount);

			value >>= (amount - 1);
			carryOut = amount & 1;
			value >>= 1;
			value |= topHalf;
		}
	}
	break;
	default:
		break;
		//TODO: Complain
	}
}

auto CPU::ArmOperation(OpCode opcode)
{
	switch (opcode >> 26 & BIT_MASK(2))
	{
	case 0x00:
	{
		if (opcode & (1 << 25))
		{
			return std::bind(&CPU::ArmDataProcessing, this,
							 ParseParams(opcode, DataProcessingSegments));
		}
		else if ((opcode & 0xFFFFFF0) == 0x12FFF10)
		{
			return std::bind(&CPU::ArmBranchAndExchange, this,
							 ParseParams(opcode, BranchAndExchangeSegments));
		}
		else if ((opcode & 0x18000F0) == 0x0000090)
		{
			return std::bind(&CPU::ArmMultiply, this,
							 ParseParams(opcode, MultiplySegments));
		}
		else if ((opcode & 0x18000F0) == 0x0800090)
		{
			return std::bind(&CPU::ArmMultiplyLong, this,
							 ParseParams(opcode, MultiplyLongSegments));
		}
		else if ((opcode & 0x1B00FF0) == 0x1000090)
		{
			return std::bind(&CPU::ArmSingleDataSwap, this,
							 ParseParams(opcode, SingleDataSwapSegments));
		}
		else if ((opcode & 0xF0) == 0xB0 || (opcode & 0xF0) == 0xD0 || (opcode & 0xF0) == 0xF0)
		{
			if (opcode & (1 << 22))
			{
				return std::bind(&CPU::ArmHalfwordDTImmOffset, this,
								 ParseParams(opcode, HalfwordDTImmOffsetSegments));
			}
			else
			{
				return std::bind(&CPU::ArmHalfwordDTRegOffset, this,
								 ParseParams(opcode, HalfwordDTRegOffsetSegments));
			}
		}
		else
		{
			return std::bind(&CPU::ArmDataProcessing, this,
							 ParseParams(opcode, DataProcessingSegments));
		}
	}
	case 0x01: // SDT and Undef
	{
		auto undefMask = 0x11 << 25 + 0x1 << 4;
		if (opcode & undefMask == undefMask)
		{
			return std::bind(&CPU::ArmUndefined, this, ParamList());
		}
		else
		{
			return std::bind(&CPU::ArmSingleDataTransfer, this,
							 ParseParams(opcode, SingleDataTransferSegments));
		}
	}
	case 0x10: // BDT and Branch
	{
		if (opcode & (1 << 25))
		{
			return std::bind(&CPU::ArmBranch, this,
							 ParseParams(opcode, BranchSegments));
		}
		else
		{
			return std::bind(&CPU::ArmBlockDataTransfer, this,
							 ParseParams(opcode, BlockDataTransferSegments));
		}
	}
	case 0x11: //CoProc and SWI
	{
		if (0xF000000 & opcode == 0xF000000)
		{
			return std::bind(&CPU::ArmSWI, this, ParamList());
		}
	}
	default:
		return std::bind(&CPU::ArmUndefined, this, ParamList());
	}
}

void CPU::ArmDataProcessing(ParamList params)
{
	std::uint32_t Op2 = params[0], Rd = params[1],
				  Rn = params[2], S = params[3],
				  OpCode = params[4], I = params[5];

	std::uint32_t Op1Val = registers.get((Register)Rn);
	auto &dest = registers.get((Register)Rd);
	auto carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
	std::uint32_t Op2Val = 0;

	//TODO: Deal with R15 operand edge case
	if (!I)
	{
		auto &Rm = registers.get((Register)(Op2 & BIT_MASK(4)));
		auto shiftType = Op2 >> 5 & BIT_MASK(2);
		auto shiftAmount = Op2 >> 7;

		if (Op2 >> 4 & BIT_MASK(1)) //Shift amount from register
		{
			if (Op2 >> 7 & BIT_MASK(1))
			{
				//TODO: Complain, should be undef or mul
			}
			shiftAmount = registers.get((Register)(shiftAmount >> 1)) & BIT_MASK(8);
		}

		Shift(Rm, shiftAmount, shiftType, carry);
		Op2Val = Rm;
	}
	else
	{
		auto Imm = Op2 & BIT_MASK(8);
		auto rotate = (Op2 >> 8) * 2;
		const uint32_t ROR = 0x11;
		Shift(Imm, rotate, ROR, carry);
		Op2Val = Imm;
	}

	auto SetFlags = [this](const uint32_t &S, const uint32_t &result, const uint8_t &carry) {
		if (S)
		{
			auto &cpsr = registers.get(CPSR);
			uint8_t zVal = result == 0;
			uint8_t nVal = result >> 31;
			SRFlag::set(cpsr, SRFlag::n, nVal);
			SRFlag::set(cpsr, SRFlag::z, zVal);
			SRFlag::set(cpsr, SRFlag::c, carry);
		}
	};

	auto &cpsr = registers.get(CPSR);

	switch ((DPOps)OpCode)
	{
	case DPOps::AND:
	{
		dest = Op1Val & Op2Val;
		break;
	}

	case DPOps::EOR:
	{
		dest = Op1Val ^ Op2Val;
		break;
	}

	case DPOps::SUB:
	{
		uint64_t result = Op1Val - Op2Val;
		dest = (uint32_t)result;
		carry = Op1Val >= Op2Val;
		auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		break;
	}
	case DPOps::RSB:
	{
		uint64_t result = Op2Val - Op1Val;
		dest = (uint32_t)result;
		carry = Op2Val >= Op1Val;
		auto overflow = ((Op1Val ^ Op2Val) & ~(Op1Val ^ dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		break;
	}

	case DPOps::ADD:
	{
		uint64_t result = Op1Val + Op2Val;
		dest = (uint32_t)result;
		carry = result >> 32;
		auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		break;
	}

	//TODO: Revisit all flag logic
	case DPOps::ADC:
	{
		uint64_t result = Op1Val + Op2Val + carry;
		dest = (uint32_t)result;
		auto overflow = ((~(Op1Val ^ Op2Val) & ((Op1Val + Op2Val) ^ Op2Val)) ^
						 (~((Op1Val + Op2Val) ^ carry) & (dest ^ carry))) >>
						31;
		carry = result >> 32;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		break;
	}

	case DPOps::SBC:
	{
		auto Op3Val = carry ^ 1;
		dest = Op1Val - Op2Val - Op3Val;
		carry = (Op1Val >= Op2Val) && ((Op1Val - Op2Val) >= (Op3Val));
		auto overflow = (((Op1Val ^ Op2Val) & ~((Op1Val - Op2Val) ^ Op2Val)) ^ ((Op1Val - Op2Val) & ~dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		break;
	}

	case DPOps::RSC:
	{
		auto Op3Val = carry ^ 1;
		dest = Op2Val - Op1Val - Op3Val;
		carry = (Op2Val >= Op1Val) && ((Op2Val - Op1Val) >= (Op3Val));
		auto overflow = (((Op2Val ^ Op1Val) & ~((Op2Val - Op1Val) ^ Op1Val)) ^ ((Op2Val - Op1Val) & ~dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		break;
	}

	case DPOps::TST:
	{
		auto result = Op1Val & Op2Val;
		SetFlags(1, result, carry);
		S = 0; //To not trigger SetFlags at bottom
		break;
	}

	case DPOps::TEQ:
	{
		auto result = Op1Val ^ Op2Val;
		SetFlags(1, result, carry);
		S = 0;
		break;
	}

	case DPOps::CMP:
	{
		auto result = Op1Val - Op2Val;
		carry = Op1Val >= Op2Val;
		SetFlags(1, result, carry);
		auto overflow = ((Op1Val ^ Op2Val) & ~(Op2Val ^ dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		S = 0;
		break;
	}

	case DPOps::CMN:
	{
		uint64_t resultBig = Op1Val + Op2Val;
		uint32_t result = (uint32_t)resultBig;
		carry = resultBig >> 32;
		SetFlags(1, result, carry);
		auto overflow = (~(Op1Val ^ Op2Val) & (Op1Val ^ dest)) >> 31;
		SRFlag::set(cpsr, SRFlag::v, overflow);
		S = 0;
		break;
	}

	case DPOps::ORR:
	{
		dest = Op1Val | Op2Val;
		break;
	}

	case DPOps::MOV:
	{
		dest = Op2Val;
		break;
	}

	case DPOps::BIC:
	{
		dest = Op2Val & ~Op2Val;
		break;
	}

	case DPOps::MVN:
	{
		dest = ~Op2Val;
		break;
	}

	default:
		break;
	}

	SetFlags(S, dest, carry);
}

void CPU::ArmMultiply(ParamList params)
{
	uint32_t Rm = params[0], Rs = params[1],
			 Rn = params[2], Rd = params[3],
			 S = params[4], A = params[5];

	auto &dest = registers.get((Register)Rd);

	if (Rd == Rm)
	{
		//TODO: Not sure but not valid
		return;
	}

	if (Rm == 15 || Rs == 15 || Rn == 15 || Rd == 15)
	{
		//TODO: Not sure, but this isnt valid!
		return;
	}

	int32_t op1 = registers.get((Register)Rm);
	int32_t op2 = registers.get((Register)Rs);
	int32_t op3 = registers.get((Register)Rn);

	if (A)
	{
		dest = op1 * op2 + op3;
	}
	else
	{
		dest = op1 * op2;
	}

	if (S)
	{
		auto &cpsr = registers.get(CPSR);
		uint8_t zVal = dest == 0;
		uint8_t nVal = dest >> 31;
		SRFlag::set(cpsr, SRFlag::n, nVal);
		SRFlag::set(cpsr, SRFlag::z, zVal);
	}
}

void CPU::ArmMultiplyLong(ParamList params)
{
	uint32_t Rm = params[0], Rs = params[1],
			 RdLo = params[2], RdHi = params[3],
			 S = params[4], A = params[5], U = params[5];

	if (RdLo == Rm || RdHi == Rm || RdLo == RdHi)
	{
		//TODO: Not sure but not valid
		return;
	}

	if (Rm == 15 || Rs == 15 || RdHi == 15 || RdLo == 15)
	{
		//TODO: Not sure, but this isnt valid!
		return;
	}

	int64_t result = 0;
	uint32_t op1 = registers.get((Register)Rm);
	uint32_t op2 = registers.get((Register)Rs);
	uint32_t op3 = registers.get((Register)RdLo);
	uint32_t op4 = registers.get((Register)RdHi);

	if (A)
	{
		result = op4;
		result <<= 32;
		result += op3;
	}

	if (U)
	{
		result += op1 * op2;
	}
	else
	{
		result += (int32_t)op1 * (int32_t)op2;
	}

	registers.get((Register)RdLo) = (uint32_t)result;
	registers.get((Register)RdHi) = result >> 32;

	if (S)
	{
		auto &cpsr = registers.get(CPSR);
		uint8_t zVal = result == 0;
		uint8_t nVal = result >> 63;
		SRFlag::set(cpsr, SRFlag::n, nVal);
		SRFlag::set(cpsr, SRFlag::z, zVal);
	}
}

void CPU::ArmSingleDataSwap(ParamList params)
{
	uint32_t Rm = params[0], Rd = params[1],
			 Rn = params[2], B = params[3];

	if (Rm == 15 || Rn == 15 || Rd == 15)
	{
		//TODO: Not sure, but this isnt valid!
		return;
	}

	if (B)
	{
		auto addr = registers.get((Register)Rn);
		auto memVal = memory->Read(Memory::AccessSize::Byte, addr, Memory::Sequentiality::NSEQ);
		memory->Write(Memory::AccessSize::Byte, addr, registers.get((Register)Rm), Memory::Sequentiality::NSEQ);
		registers.get((Register)Rd) = memVal;
	}
	else
	{
		auto addr = registers.get((Register)Rn);
		auto memVal = memory->Read(Memory::AccessSize::Word, addr, Memory::Sequentiality::NSEQ);
		//TODO: Maybe these writes need to be word aligned?
		memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm), Memory::Sequentiality::NSEQ);
		memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm), Memory::Sequentiality::NSEQ);
		memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm), Memory::Sequentiality::NSEQ);
		memory->Write(Memory::AccessSize::Word, addr, registers.get((Register)Rm), Memory::Sequentiality::NSEQ);
		registers.get((Register)Rd) = memVal;
	}
}

void CPU::ArmBranchAndExchange(ParamList params)
{
	uint32_t Rn = params[0];
	if (Rn & BIT_MASK(1))
	{
		SRFlag::set(registers.get(CPSR), SRFlag::thumb, 1);
	}
	else
	{
		SRFlag::set(registers.get(CPSR), SRFlag::thumb, 0);
	}
	
	registers.get(Register::R15) = registers.get((Register)Rn);
	PipelineFlush();
}

void CPU::ArmHalfwordDTRegOffset(ParamList params)
{
	return;
}

void CPU::ArmHalfwordDTImmOffset(ParamList params)
{
	return;
}

void CPU::ArmSingleDataTransfer(ParamList params)
{
	uint32_t Offset = params[0], Rd = params[1], Rn = params[2],
			 L = params[3], W = params[4], B = params[5],
			 U = params[6], P = params[7], I = params[8];

	if (I)
	{
		auto carry = SRFlag::get(registers.get(CPSR), SRFlag::c);
		auto &Rm = registers.get((Register)(Offset & BIT_MASK(4)));
		auto shiftType = Offset >> 5 & BIT_MASK(2);
		auto shiftAmount = Offset >> 7;
		Shift(Rm, shiftAmount, shiftType, carry);
		Offset = Rm;
	}

	auto base = registers.get((Register)Rn);
	auto baseOffset = base;

	if (U)
	{
		baseOffset += Offset;
	}
	else
	{
		baseOffset -= Offset;
	}

	auto memAddr = base;
	if (P)
	{
		memAddr = baseOffset;
	}

	auto destReg = registers.get((Register)Rd);

	if (L)
	{
		if (B)
		{
			destReg = memory->Read(Memory::AccessSize::Byte, memAddr, Memory::Sequentiality::NSEQ);
		}
		else
		{
			//TODO: Check word boundary addr LD behaviour, seems complicated pg 49 or 55 of pdf
			destReg = memory->Read(Memory::AccessSize::Word, memAddr, Memory::Sequentiality::NSEQ);
		}
	}
	else
	{
		if (B)
		{
			memory->Write(Memory::AccessSize::Byte, memAddr, destReg, Memory::Sequentiality::NSEQ);
			memory->Write(Memory::AccessSize::Byte, memAddr + 1, destReg, Memory::Sequentiality::NSEQ);
			memory->Write(Memory::AccessSize::Byte, memAddr + 2, destReg, Memory::Sequentiality::NSEQ);
			memory->Write(Memory::AccessSize::Byte, memAddr + 3, destReg, Memory::Sequentiality::NSEQ);
		}
		else
		{
			//TODO: Check word boundary addr
			memory->Write(Memory::AccessSize::Word, memAddr, destReg, Memory::Sequentiality::NSEQ);
		}
	}

	if (W || !P)
	{
		registers.get((Register)Rn) = baseOffset;
	}
}

void CPU::ArmUndefined(ParamList params)
{
	return;
}

void CPU::ArmBlockDataTransfer(ParamList params)
{
	return;
}

void CPU::ArmBranch(ParamList params)
{

}

void CPU::ArmSWI(ParamList params)
{
	return;
}
} // namespace ARM7TDMI