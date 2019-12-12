#include "arm7tdmi/cpu.hpp"
#include <unistd.h>
#include <iostream>
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace ARM7TDMI {
void CPU::Execute() {
  auto opcode = pipeline[0];
  auto& pc = registers.get(R15);

  auto Operation = ArmOperation(opcode);
  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	spdlog::get("std")->debug("PC:{:X} - Op:{:X} - NZCV {:b}", pc - 2, opcode,
	                          SRFlag::get(registers.get(CPSR), SRFlag::flags));
	backtrace.addOpPCPair(pc - 2, opcode);

	pc &= ~1;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 2;
	pipeline[1] = memory->Read(Half, pc, SEQ);

	Operation = ThumbOperation(opcode);
  } else {
	spdlog::get("std")->debug("PC:{:X} - Op:{:X} - NZCV {:04b}", pc - 4, opcode,
	                          SRFlag::get(registers.get(CPSR), SRFlag::flags));
	backtrace.addOpPCPair(pc - 4, opcode);

	pc &= ~3;

	pipeline[0] = pipeline[1];
	// TODO: Fix memory sequentiality
	pc += 4;
	pipeline[1] = memory->Read(Word, pc, SEQ);
  }

  if (!HandleInterruptRequests()) {
	Operation();
  }

  spdlog::get("std")->trace("************************");
}

StateView CPU::ViewState() {
  RegisterView regView;
  for (int i = 0; i < 16; i++) {
	regView[i] = registers.view((Register)i);
  }

  StateView stateView{regView, backtrace};
  return stateView;
}

bool CPU::HandleInterruptRequests() {
  auto& cpsr = registers.get(CPSR);
  if (SRFlag::get(cpsr, SRFlag::irqDisable)) {
	return false;
  }
  auto ie = memory->Read(Half, IE, NSEQ);
  auto irf = memory->Read(Half, IF, NSEQ);
  auto ime = memory->Read(Half, IME, NSEQ);

  if (ime && (ie & irf)) {
	spdlog::get("std")->debug("IRQ Successful {:B} from {:X}", (ie & irf),
	                          registers.get(R15));
	registers.switchMode(SRFlag::ModeBits::IRQ);

	registers.get(R14) = registers.get(R15);
	if (SRFlag::get(cpsr, SRFlag::thumb)) {
	  SRFlag::set(cpsr, SRFlag::thumb, 0);
	}

	SRFlag::set(cpsr, SRFlag::irqDisable, 1);
	registers.get(R15) = 0x18;
	PipelineFlush();
	return true;
  }
  return false;
}

void CPU::PipelineFlush() {
  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
	auto& pc = registers.get(R15);
	pc &= ~1;
	spdlog::get("std")->trace("Flush to THUMB {:X}", pc);
	pipeline[0] = memory->Read(Half, pc, NSEQ);
	pc += 2;
	pipeline[1] = memory->Read(Half, pc, SEQ);
  } else {
	auto& pc = registers.get(R15);
	pc &= ~3;
	spdlog::get("std")->trace("Flush to ARM {:X}", pc);
	pipeline[0] = memory->Read(Word, pc, NSEQ);
	pc += 4;
	pipeline[1] = memory->Read(Word, pc, SEQ);
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