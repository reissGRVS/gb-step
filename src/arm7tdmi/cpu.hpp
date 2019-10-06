#pragma once

#include <array>
#include <cstdint>


namespace ARM7TDMI
{

//Registers and Conditions

	// https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
	enum Condition 
	{
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
	enum Register 
	{
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
		R15 = 15,
		CPSR = 16,
		SPSR = 17
	};

	namespace SRFlag
	{
		struct BitLocation
		{
			const uint8_t bit;
			const uint8_t size;
		};

		enum ModeBits 
		{
			USR = 0x10,
			FIQ = 0x11,
			IRQ = 0x12,
			SVC = 0x13,
			ABT = 0x17,
			UND = 0x1B,
			SYS = 0x1F
		};

		const BitLocation modeBits = {0,5};
		const BitLocation thumb = {5,1};
		const BitLocation fiqDisable = {6,1};
		const BitLocation irqDisable = {7,1};
		const BitLocation q = {27,1};
		const BitLocation v = {28,1};
		const BitLocation c = {29,1};
		const BitLocation z = {30,1};
		const BitLocation n = {31,1};
		
		uint8_t get(uint32_t sr, const BitLocation& flag)
		{
			uint32_t mask = (1 << flag.size) - 1;
			return (sr >> flag.bit) & mask;
		}

		void set(uint32_t sr, const BitLocation& flag, uint8_t val)
		{
			uint32_t mask = (1 << flag.size) - 1;
			if ((mask & val) != val)
			{
				//TODO: Complain about invalid value
			}
			mask = ~mask;
			sr &= mask;
			sr |= val;
		}
	}

	enum ModeBank 
	{
		SYS = 0,
		FIQ = 1,
		SVC = 2,
		ABT = 3,
		IRQ = 4,
		UND = 5,
	};

	class RegisterSet
	{
		public:
			std::uint32_t& get(ModeBank mode, Register reg)
			{
				if (reg <= Register::R12)
				{
					if (mode == ModeBank::FIQ && reg >= Register::R8)
					{
						return registers.GPR_FIQ[reg - Register::R8];
					}
					else
					{
						return registers.GPR[(uint8_t)reg];
					}
				}
				else if (reg == Register::R13)
				{
					return registers.SP[(uint8_t)mode];
				}
				else if (reg == Register::R14)
				{
					return registers.LR[(uint8_t)mode];
				}
				else if (reg == Register::R15)
				{
					return registers.PC;
				}
				else if (reg == Register::CPSR)
				{
					return registers.CPSR;
				}
				else if (reg == Register::SPSR)
				{
					if (mode == ModeBank::SYS)
					{
						//TODO: Throw error
					}
					return registers.SPSR[(uint8_t)mode - 1];
				}
				
			}
			
			// https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
			bool ConditionCheck(Condition cond)
			{
				switch (cond)
				{
				case EQ: return SRFlag::get(CPSR,SRFlag::z);
				case NE: return !SRFlag::get(CPSR,SRFlag::z);
				case CS: return SRFlag::get(CPSR,SRFlag::c);
				case CC: return !SRFlag::get(CPSR,SRFlag::c);
				case MI: return SRFlag::get(CPSR,SRFlag::n);
				case PL: return !SRFlag::get(CPSR,SRFlag::n);
				case VS: return SRFlag::get(CPSR,SRFlag::v);
				case VC: return !SRFlag::get(CPSR,SRFlag::v);
				case HI: return SRFlag::get(CPSR,SRFlag::c) && !SRFlag::get(CPSR,SRFlag::z);
				case LS: return !SRFlag::get(CPSR,SRFlag::c) && SRFlag::get(CPSR,SRFlag::z);
				case GE: return SRFlag::get(CPSR,SRFlag::n) == SRFlag::get(CPSR, SRFlag::v);
				case LT: return SRFlag::get(CPSR,SRFlag::n) != SRFlag::get(CPSR, SRFlag::v);
				case GT: return SRFlag::get(CPSR,SRFlag::n) == SRFlag::get(CPSR, SRFlag::v) && !SRFlag::get(CPSR,SRFlag::z);
				case LE: return SRFlag::get(CPSR,SRFlag::n) != SRFlag::get(CPSR, SRFlag::v) && SRFlag::get(CPSR,SRFlag::z);
				case AL: return true;
				case NV: return false;
				default: //TODO Complain 
					return false;
				}
			}
		private:
			struct Registers
			{
				std::array<std::uint32_t,13> GPR{}; 	//R0-12 
				std::array<std::uint32_t,5> GPR_FIQ{}; 	//R8-12 FIQ
				std::array<std::uint32_t,6> SP{}; 		//R13 All modes
				std::array<std::uint32_t,6> LR{};		//R14 All modes
				std::uint32_t PC;						//R15
				std::uint32_t CPSR;	
				std::array<std::uint32_t,5> SPSR{}; 	//System/Usr mode has no SPSR
			} registers;
	};

//CPU
	class CPU
	{
		public:
			RegisterSet registers;
	};

}