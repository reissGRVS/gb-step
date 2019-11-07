#include "cpu.hpp"
#include <unistd.h>
#include <iostream>
#include "../utils.hpp"
#include "spdlog/spdlog.h"

int count = 0;

namespace ARM7TDMI {
std::uint32_t CPU::Execute() {
  HandleInterruptRequests();
  auto opcode = pipeline[0];
  count++;

  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	spdlog::debug("{:X} - PC:{:X} - Op:{:X}", count, registers.get(R15) - 2,
	              opcode);
	auto& pc = registers.get(R15);
	pc &= ~1;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 2;
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);

	ThumbOperation(opcode)();
  } else {
	spdlog::debug("{:X} - PC:{:X} - Op:{:X}", count, registers.get(R15) - 4,
	              opcode);
	auto& pc = registers.get(R15);
	pc &= ~3;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);

	ArmOperation(opcode)();
  }

  //   if (registers.get(R1) == 0 && registers.get(R2) == 1 &&
  //       registers.get(R3) == 3) {
  // 	slow = true;
  //   }

  if (slow) {
	usleep(100000);
	spdlog::set_level(spdlog::level::debug);
	spdlog::debug("********************************");
  }

  return 1;
}

void CPU::HandleInterruptRequests() {
  auto& cpsr = registers.get(CPSR);
  if (SRFlag::get(cpsr, SRFlag::irqDisable)) {
	return;
  }
  auto ie =
      memory->Read(Memory::AccessSize::Half, IE, Memory::Sequentiality::NSEQ);
  auto irf =
      memory->Read(Memory::AccessSize::Half, IF, Memory::Sequentiality::NSEQ);
  auto ime =
      memory->Read(Memory::AccessSize::Half, IME, Memory::Sequentiality::NSEQ);

  if (ime && (ie & irf)) {
	spdlog::info("IRQ Successful");
	registers.switchMode(SRFlag::ModeBits::IRQ);

	if (SRFlag::get(cpsr, SRFlag::thumb)) {
	  registers.get(R14) = registers.get(R15);
	  SRFlag::set(cpsr, SRFlag::thumb, 0);
	} else {
	  registers.get(R14) = registers.get(R15) - 4;
	}

	SRFlag::set(cpsr, SRFlag::irqDisable, 1);
	registers.get(R15) = 0x18;
	PipelineFlush();
  }
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
	    (opcode >> it->second) & NBIT_MASK((it->first - it->second + 1));
	params.push_back(param);
  }
  return params;
}

}  // namespace ARM7TDMI