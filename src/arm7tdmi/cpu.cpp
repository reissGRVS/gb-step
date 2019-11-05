#include "cpu.hpp"
#include <iostream>
#include "../utils.hpp"
#include "spdlog/spdlog.h"

int count = 0;

namespace ARM7TDMI {
void CPU::Execute() {
  auto opcode = pipeline[0];
  count++;
  spdlog::debug("{:X} - {:X}", registers.get(R15) - 4, opcode);
  if (!(count % 1000)) {
	spdlog::info("{:X}", count);
  }
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

	ThumbOperation(opcode)();
  } else {
	auto& pc = registers.get(R15);
	pc &= ~3;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);

	ArmOperation(opcode)();
  }

  auto LR = registers.get(R14);
  spdlog::debug("LR: {:X}", LR);
}

void CPU::PipelineFlush() {
  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	spdlog::debug("Flush to THUMB");
	auto& pc = registers.get(R15);
	pc &= ~1;
	pipeline[0] = memory->Read(Memory::Half, pc, Memory::NSEQ);
	pc += 2;
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);
  } else {
	spdlog::debug("Flush to ARM");
	auto& pc = registers.get(R15);
	pc &= ~3;
	pipeline[0] = memory->Read(Memory::Word, pc, Memory::NSEQ);
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);
  }
}

ParamList CPU::ParseParams(OpCode opcode, ParamSegments paramSegs) {
  ParamList params;
  for (auto it = paramSegs.rbegin(); it != paramSegs.rend(); ++it) {
	auto param =
	    (opcode >> it->second) & BIT_MASK((it->first - it->second + 1));
	params.push_back(param);
  }
  return params;
}

}  // namespace ARM7TDMI