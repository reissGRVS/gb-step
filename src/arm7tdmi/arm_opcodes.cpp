#include "cpu.hpp"

#define BIT_MASK(N) ((1<<N)-1)

namespace ARM7TDMI
{
	ParamList ParseParams(OpCode opcode, ParamSegments paramSegs)
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

	std::function<void(ParamList)> ArmOperation(OpCode opcode)
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

}