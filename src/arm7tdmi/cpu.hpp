#pragma once

#include <array>
#include <cstdint>


namespace ARM7TDMI
{

enum Mode {
	USR = 0x10,
	FIQ = 0x11,
	IRQ = 0x12,
	SVC = 0x13,
	ABT = 0x17,
	UND = 0x1B,
	SYS = 0x1F
};

enum ModeBank {
	SYS = 0,
	FIQ = 1,
	SVC = 2,
	ABT = 3,
	IRQ = 4,
	UND = 5,
};

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
	R15 = 15,
	CPSR = 16,
	SPSR = 17
};

class RegisterSet
{
	public:
		std::uint32_t& getReg(ModeBank mode, Register reg)
		{
			if (reg <= Register::R12)
			{
				if (reg >= Register::R8 && mode == ModeBank::FIQ)
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

class CPU
{
	public:
		RegisterSet registers;
};

}