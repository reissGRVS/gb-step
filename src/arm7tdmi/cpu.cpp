#include "arm7tdmi/cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <iostream>
#include <unistd.h>

namespace ARM7TDMI {

void CPU::Reset() {
  registers.get(Register::R15) = 0x00000000;

  PipelineFlush();
}

void CPU::Execute() {
  if (halt) {
    clock->Tick(1);
    return;
  }

  if (interruptReady && HandleInterruptRequests()) {
    return;
  }

  auto opcode = pipeline[0];
  auto &pc = registers.get(R15);

  if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
    backtrace.addOpPCPair(pc - 2, opcode);

    pc &= ~1;

    pipeline[0] = pipeline[1];
    pc += 2;
    pipeline[1] = memory->Read(Half, pc, SEQ);

    ThumbOperation(opcode)();
  } else {
    backtrace.addOpPCPair(pc - 4, opcode);

    pc &= ~3;

    pipeline[0] = pipeline[1];
    pc += 4;
    pipeline[1] = memory->Read(Word, pc, SEQ);
    ArmOperation(opcode)();
  }
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
  auto &cpsr = registers.get(CPSR);
  if (interruptReady && !SRFlag::get(cpsr, SRFlag::irqDisable)) {

    registers.switchMode(SRFlag::ModeBits::IRQ);
    registers.get(R14) = registers.get(R15);
    if (SRFlag::get(cpsr, SRFlag::thumb)) {
      registers.get(R14) += 2;
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
    auto &pc = registers.get(R15);
    pc &= ~1;

    pipeline[0] = memory->Read(Half, pc, NSEQ);
    pc += 2;
    pipeline[1] = memory->Read(Half, pc, SEQ);
  } else {
    auto &pc = registers.get(R15);
    pc &= ~3;

    pipeline[0] = memory->Read(Word, pc, NSEQ);
    pc += 4;
    pipeline[1] = memory->Read(Word, pc, SEQ);
  }
}

void CPU::ParseParams(OpCode opcode, const ParamSegments& paramSegs) {
  U16 index = 0;
  for (auto it = paramSegs.rbegin(); it != paramSegs.rend(); ++it) {
    auto param = BIT_RANGE(opcode, it->second, it->first);
    params[index] = param;
	index++;
  }
}

} // namespace ARM7TDMI