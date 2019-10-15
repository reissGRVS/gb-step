#include "cpu.hpp"

namespace ARM7TDMI
{
void CPU::Execute()
{
	auto opcode = pipeline[0];

	if (SRFlag::get(registers.get(CPSR), SRFlag::thumb))
	{
		auto &pc = registers.get(R15);
		pc &= ~1;

		pipeline[0] = pipeline[1];
		//TODO: Fix memory sequentiality
		pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);
		//TODO: run thumb opcode
	}
	else
	{
		auto &pc = registers.get(R15);
		pc &= ~3;

		pipeline[0] = pipeline[1];
		//TODO: Fix memory sequentiality
		pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);
		//TODO: run arm opcode
	}
}
} // namespace ARM7TDMI