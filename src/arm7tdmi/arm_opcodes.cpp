#include "cpu.hpp"

#define BIT_MASK(N) ((1<<N)-1)

namespace ARM7TDMI
{
	ParamList CPU::ParseParams(OpCode opcode, ParamSegments paramSegs)
	{
		ParamList params;
		int startPos = 0;
		for (auto it = paramSegs.rbegin(); it != paramSegs.rend(); ++it)
		{
			opcode >>= (it->second - startPos);
			params.push_back(opcode & BIT_MASK(it->second-it->first+1 ));
		}
		return params;
	}

	void CPU::Shift(std::uint32_t& value, std::uint32_t amount, const std::uint32_t& shiftType, std::uint8_t& carryOut)
	{
		switch (shiftType)
		{
		case 0x00: //LSL
			value <<= (amount-1);
			carryOut = value >> 31;
			value <<= 1;
			break;
		case 0x01: //LSR
		case 0x10: //ASR
			auto neg = amount >> 31;
			if (amount == 0)
			{
				amount = 32;
			}
			value >>= (amount-1);
			carryOut = amount & 1;
			value >>= 1;

			if (shiftType == 0x10 && neg)
			{
				value |= (BIT_MASK(amount) << (32-amount));
			}
			break;
		case 0x11: //RR
			
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
				auto topHalf = value << (32-amount);

				value >>= (amount-1);
				carryOut = amount & 1;
				value >>= 1;
				value |= topHalf;
			}
			break;
		default:
			//TODO: Complain
		}
	}

	std::function<void(ParamList)> CPU::ArmOperation(OpCode opcode)
	{
		switch (opcode >> 26 & BIT_MASK(2))
		{
		case 0x00:
			if (opcode & (1 << 25))
			{
				return std::bind(ArmDataProcessing,
								ParseParams(opcode, DataProcessingSegments));
			}
    		else if ((opcode & 0xFFFFFF0) == 0x12FFF10) 
			{
				return std::bind(ArmBranchAndExchange,
								ParseParams(opcode, BranchAndExchangeSegments));
    		} 
			else if ((opcode & 0x18000F0) == 0x0000090) 
			{
				return std::bind(ArmMultiply,
								ParseParams(opcode, MultiplySegments));
			}
			else if ((opcode & 0x18000F0) == 0x0800090) 
			{
				return std::bind(ArmMultiplyLong,
								ParseParams(opcode, MultiplyLongSegments));
    		} 
			else if ((opcode & 0x1B00FF0) == 0x1000090) 
			{
				return std::bind(ArmSingleDataSwap,
								ParseParams(opcode, SingleDataSwapSegments));
			} 
			else if ((opcode & 0xF0) == 0xB0 || (opcode & 0xF0) == 0xD0 || (opcode & 0xF0) == 0xF0) 
			{
				if (opcode & (1 << 22))
				{
					return std::bind(ArmHalfwordDTImmOffset,
								ParseParams(opcode, HalfwordDTImmOffsetSegments));
				}
				else
				{
					return std::bind(ArmHalfwordDTRegOffset,
								ParseParams(opcode, HalfwordDTRegOffsetSegments));
				}
			}
			else
			{
				return std::bind(ArmDataProcessing,
								ParseParams(opcode, DataProcessingSegments));
			}
		case 0x01: // SDT and Undef
			auto undefMask = 0x11 << 25 + 0x1 << 4;
			if (opcode & undefMask == undefMask)
			{
				return std::bind(ArmUndefined, ParamList());
			}
			else
			{
				return std::bind(ArmSingleDataTransfer, 
								ParseParams(opcode, SingleDataTransferSegments));
			}
		case 0x10: // BDT and Branch
			if (opcode & (1 << 25))
			{
				return std::bind(ArmBranch, 
								ParseParams(opcode, BranchSegments));
			}
			else
			{
				return std::bind(ArmBlockDataTransfer, 
								ParseParams(opcode, BlockDataTransferSegments));
			}
		case 0x11: //CoProc and SWI
			if (0xF000000 & opcode == 0xF000000)
			{
				return std::bind(ArmSWI, ParamList());
			}
		default:
			return std::bind(ArmUndefined, ParamList());
		}
		
	}

	void CPU::ArmDataProcessing(ParamList params)
	{
		std::uint32_t 	Op2 = params[0], 	Rd = params[1], 
						Rn = params[2], 	S = params[3], 
						OpCode = params[4], I = params[5];

		std::uint32_t Op1Val = registers.get((Register)Rn);
		auto & dest = registers.get((Register)Rd);
		std::uint8_t carry = 0;
		std::uint32_t Op2Val = 0;

		if (!I)
		{
			auto& Rm = registers.get((Register)(Op2 & BIT_MASK(4)));
			auto shiftType = Op2 >> 5 & BIT_MASK(2);
			auto shiftAmount = Op2 >> 7;
			
			if (Op2 >> 4 & BIT_MASK(1)) //Shift amount from register
			{
				//TODO: Check If bit7 is 1 as should be undef or multiply
				shiftAmount = registers.get((Register)(shiftAmount >> 1)) & BIT_MASK(8);
			}

			Shift(Rm, shiftAmount, shiftType, carry);
			Op2Val = Rm;
		}
		else
		{
			auto Imm = Op2 & BIT_MASK(8);
			auto rotate = (Op2 >> 8) * 2;
			Shift(Imm, rotate, 0x11, carry);
			Op2Val = Imm;
		}
		 
		//TODO: Flags side effects, use right carry
		switch ((DPOps)OpCode)
		{
		case DPOps::AND:
			dest = Op1Val & Op2Val;
			break;
		case DPOps::EOR:
			dest = Op1Val ^ Op2Val;
			break;
		case DPOps::SUB:
			dest = Op1Val - Op2Val;
			break;
		case DPOps::RSB:
			dest = Op2Val - Op1Val;
			break;
		case DPOps::ADD:
			dest = Op1Val + Op2Val;
			break;
		case DPOps::ADC:
			dest = Op1Val + Op2Val + carry;
			break;
		case DPOps::SBC:
			dest = Op1Val - Op2Val + carry - 1;
			break;
		case DPOps::RSC:
			dest = Op2Val - Op1Val + carry - 1;
			break;
		case DPOps::TST:
			//AND with no write
			break;
		case DPOps::TEQ:
			//EOR with no write
			break;
		case DPOps::CMP:
			//SUB with no write
			break;
		case DPOps::CMN:
			//ADD with no write
			break;
		case DPOps::ORR:
			dest = Op1Val | Op2Val;
			break;
		case DPOps::MOV:
			dest = Op2Val;
			break;
		case DPOps::BIC:
			dest = Op2Val & ~Op2Val;
			break;
		case DPOps::MVN:
			dest = ~Op2Val;
			break;
		
		default:
			break;
		}
	}

}