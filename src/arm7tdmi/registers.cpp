#include "arm7tdmi/registers.hpp"
#include "spdlog/spdlog.h"

namespace ARM7TDMI {

RegisterSet::ModeBank RegisterSet::getModeBank(ModeBits modeBits)
{
	switch (modeBits) {
	case ModeBits::USR:
		return RegisterSet::ModeBank::SYS;
	case ModeBits::FIQ:
		return RegisterSet::ModeBank::FIQ;
	case ModeBits::IRQ:
		return RegisterSet::ModeBank::IRQ;
	case ModeBits::SVC:
		return RegisterSet::ModeBank::SVC;
	case ModeBits::ABT:
		return RegisterSet::ModeBank::ABT;
	case ModeBits::UND:
		return RegisterSet::ModeBank::UND;
	case ModeBits::SYS:
		return RegisterSet::ModeBank::SYS;
	default:
		spdlog::get("std")->error("Invalid modeBits passed");
		exit(-1);
	}
}

U32& RegisterSet::get(Register reg)
{
	return registers.ACTIVE[reg];
}

bool RegisterSet::ConditionCheck(Condition cond)
{
	switch (cond) {
	case EQ:
		return CPSR.z;
	case NE:
		return !CPSR.z;
	case CS:
		return CPSR.c;
	case CC:
		return !CPSR.c;
	case MI:
		return CPSR.n;
	case PL:
		return !CPSR.n;
	case VS:
		return CPSR.v;
	case VC:
		return !CPSR.v;
	case HI:
		return CPSR.c && !CPSR.z;
	case LS:
		return !CPSR.c || CPSR.z;
	case GE:
		return CPSR.n == CPSR.v;
	case LT:
		return CPSR.n != CPSR.v;
	case GT:
		return (CPSR.n == CPSR.v) && !CPSR.z;
	case LE:
		return CPSR.n != CPSR.v || CPSR.z;
	case AL:
		return true;
	case NV:
		return false;
	default:
		spdlog::get("std")->error("Out of range register condition check");
		return false;
	}
}

void RegisterSet::SwitchMode(ModeBits mode)
{
	auto newBank = getModeBank(mode);

	if (newBank == currentBank)
		return;

	if (newBank != ModeBank::SYS) {
		SPSR[(uint8_t)newBank - 1] = CPSR;
	}
	//Update R13 and R14
	registers.SP[(uint8_t)currentBank] = registers.ACTIVE[13];
	registers.ACTIVE[13] = registers.SP[(uint8_t)newBank];

	registers.LR[(uint8_t)currentBank] = registers.ACTIVE[14];
	registers.ACTIVE[14] = registers.LR[(uint8_t)newBank];

	if (newBank == ModeBank::FIQ)
	{
		for (U16 i = 0; i < registers.GPR.size(); i++)
		{
			registers.GPR[i] = registers.ACTIVE[i+8];
			registers.ACTIVE[i+8] = registers.GPR_FIQ[i];
		}
	}
	else if (currentBank == ModeBank::FIQ)
	{
		for (U16 i = 0; i < registers.GPR.size(); i++)
		{
			registers.GPR_FIQ[i] = registers.ACTIVE[i+8];
			registers.ACTIVE[i+8] = registers.GPR[i];
		}
	}

	currentBank = newBank;
	CPSR.modeBits = mode;
}

StatusRegister& RegisterSet::GetSPSR()
{
	if (currentBank != ModeBank::SYS) {
		return SPSR[(uint8_t)currentBank - 1];
	}
	else
	{
		spdlog::get("std")->error("Tried to access Sys SPSR @ PC {:X}", registers.ACTIVE[15]);
		return CPSR;
	}
}

} // namespace ARM7TDMI
