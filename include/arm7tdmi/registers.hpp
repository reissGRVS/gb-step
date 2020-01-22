#pragma once

#include <array>
#include "int.hpp"
#include <memory>

#include "memory/memory.hpp"

namespace ARM7TDMI {
// https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
enum Condition {
	EQ = 0x0,
	NE = 0x1,
	CS = 0x2,
	CC = 0x3,
	MI = 0x4,
	PL = 0x5,
	VS = 0x6,
	VC = 0x7,
	HI = 0x8,
	LS = 0x9,
	GE = 0xA,
	LT = 0xB,
	GT = 0xC,
	LE = 0xD,
	AL = 0xE,
	NV = 0xF
};

// https://problemkaputt.de/gbatek.htm#armcpuregisterset
enum Register {
	R0 = 0,
	R1 = 1,
	R2 = 2,
	R3 = 3,
	R4 = 4,
	R5 = 5,
	R6 = 6,
	R7 = 7,
	R8 = 8,
	R9 = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15
};

enum ModeBits {
	USR = 0x10,
	FIQ = 0x11,
	IRQ = 0x12,
	SVC = 0x13,
	ABT = 0x17,
	UND = 0x1B,
	SYS = 0x1F
};

struct StatusRegister {
	U32 ToU32(){
		U32 registerValue = modeBits
		+ (thumb << 5)
		+ (fiqDisable << 6)
		+ (irqDisable << 7)
		+ (v << 28)
		+ (c << 29)
		+ (z << 30)
		+ (n << 31);

		return registerValue;
	}

	void FromU32(U32 value)
	{
		modeBits = (ModeBits)BIT_RANGE(value, 0, 4);
		thumb = BIT_RANGE(value, 5, 5);
		fiqDisable = BIT_RANGE(value, 6, 6);
		irqDisable = BIT_RANGE(value, 7, 7);
		v = BIT_RANGE(value, 28, 28);
		c = BIT_RANGE(value, 29, 29);
		z = BIT_RANGE(value, 30, 30);
		n = BIT_RANGE(value, 31, 31);
	}

	void FlagsFromU4(U8 value)
	{
		v = BIT_RANGE(value, 0, 0);
		c = BIT_RANGE(value, 1, 1);
		z = BIT_RANGE(value, 2, 2);
		n = BIT_RANGE(value, 3, 3);
	}

	ModeBits modeBits;
	bool thumb;
	bool fiqDisable;
	bool irqDisable;
	bool v;
	bool c;
	bool z;
	bool n;
};



class RegisterSet {
public:
	enum ModeBank {
		SYS = 0,
		FIQ = 1,
		SVC = 2,
		ABT = 3,
		IRQ = 4,
		UND = 5
	};

	ModeBank getModeBank(ModeBits modeBits);

	U32& get(Register reg);

	// https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
	bool ConditionCheck(Condition cond);

	void SwitchMode(ModeBits mode);


	StatusRegister CPSR;
	StatusRegister& GetSPSR();

private:
	ModeBank currentBank = ModeBank::SYS;

	std::array<StatusRegister, 5> SPSR{};
	struct Registers {
		std::array<U32, 16> ACTIVE{};
		std::array<U32, 5> GPR{}; // R8-12 Other modes
		std::array<U32, 5> GPR_FIQ{}; // R8-12 FIQ
		std::array<U32, 6> SP{}; // R13 All modes
		std::array<U32, 6> LR{}; // R14 All modes// System/Usr mode has no SPSR
	} registers;
};

} // namespace ARM7TDMI