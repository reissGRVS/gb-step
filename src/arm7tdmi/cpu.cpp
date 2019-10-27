#include "cpu.hpp"
#include <iostream>
#include "spdlog/spdlog.h"

int count = 0;

namespace ARM7TDMI {
void CPU::Execute() {
  auto opcode = pipeline[0];
  count++;
  spdlog::debug("{:X} - {:X}", registers.get(R15) - 4, opcode);

  if (registers.get(R15) > 0x4000) {
	std::cout << "Instructions: " << count << std::endl;
	exit(0);
  }
  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	auto& pc = registers.get(R15);
	pc &= ~1;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 2;
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);

	// TODO: run thumb opcode
  } else {
	auto& pc = registers.get(R15);
	pc &= ~3;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);

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
  } else {
	auto& pc = registers.get(R15);
	pc &= ~3;
	pipeline[0] = memory->Read(Memory::Word, pc, Memory::NSEQ);
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);
  }
}

}  // namespace ARM7TDMI