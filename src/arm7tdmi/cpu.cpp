#include "cpu.hpp"

namespace ARM7TDMI {
void CPU::Execute() {
  auto opcode = pipeline[0];

  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	auto& pc = registers.get(R15);
	pc &= ~1;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);
	pc += 2;
	// TODO: run thumb opcode
  } else {
	auto& pc = registers.get(R15);
	pc &= ~3;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);
	pc += 4;
	auto op = ArmOperation(opcode);
	op();
  }
}

void CPU::PipelineFlush() {
  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	auto& pc = registers.get(R15);
	pc &= ~1;
	pipeline[0] = memory->Read(Memory::Half, pc, Memory::NSEQ);
	pc += 2;
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);
	pc += 2;
  } else {
	auto& pc = registers.get(R15);
	pc &= ~3;
	pipeline[0] = memory->Read(Memory::Word, pc, Memory::NSEQ);
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);
	pc += 4;
  }
}

}  // namespace ARM7TDMI