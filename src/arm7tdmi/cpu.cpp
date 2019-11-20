#include "arm7tdmi/cpu.hpp"
#include <unistd.h>
#include <iostream>
#include "spdlog/spdlog.h"
#include "utils.hpp"

int count = 0;

namespace ARM7TDMI {
std::uint32_t CPU::Execute() {
  HandleInterruptRequests();
  auto opcode = pipeline[0];
  auto& pc = registers.get(R15);

  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	spdlog::get("std")->debug("PC:{:X} - Op:{:X}", pc - 2, opcode);
	backtrace.addOpPCPair(pc - 2, opcode);

	pc &= ~1;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 2;
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);

	ThumbOperation(opcode)();
  } else {
	spdlog::get("std")->debug("PC:{:X} - Op:{:X}", registers.get(R15) - 4,
	                          opcode);
	backtrace.addOpPCPair(pc - 4, opcode);

	pc &= ~3;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 4;
	pipeline[1] = memory->Read(Memory::Word, pc, Memory::NSEQ);

	ArmOperation(opcode)();
  }

  spdlog::get("std")->trace("************************");
  return 1;
}

StateView CPU::ViewState() {
  RegisterView regView;
  for (int i = 0; i < 16; i++) {
	regView[i] = registers.view((Register)i);
  }

  StateView stateView{regView, backtrace};
  return stateView;
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
	spdlog::get("std")->info("IRQ Successful");
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
	spdlog::get("std")->trace("Flush to THUMB");
	auto& pc = registers.get(R15);
	pc &= ~1;
	pipeline[0] = memory->Read(Memory::Half, pc, Memory::NSEQ);
	pc += 2;
	pipeline[1] = memory->Read(Memory::Half, pc, Memory::NSEQ);
  } else {
	spdlog::get("std")->trace("Flush to ARM");
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
	auto param = BIT_RANGE(opcode, it->second, it->first);
	params.push_back(param);
  }
  return params;
}

}  // namespace ARM7TDMI