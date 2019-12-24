#include "arm7tdmi/cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <iostream>
#include <unistd.h>

namespace ARM7TDMI {

void CPU::Reset()
{
	// Skip BIOS
	registers.get(ModeBank::SVC, Register::R13) = 0x03007FE0;
	registers.get(ModeBank::IRQ, Register::R13) = 0x03007FA0;
	registers.get(Register::R13) = 0x03007F00;
	registers.switchMode(SRFlag::ModeBits::USR);
	registers.get(Register::R15) = 0x08000000;

	PipelineFlush();
}

void CPU::Execute()
{
	auto opcode = pipeline[0];
	auto& pc = registers.get(R15);

	auto Operation = ArmOperation(opcode);
	if (SRFlag::get(registers.get(CPSR), SRFlag::thumb)) {
		spdlog::get("std")->debug(
			"PC:{:X} - Op:{:X} - R0:{:X} - R1:{:X} - R2:{:X} - R3:{:X} - R4:{:X}",
			pc - 2, opcode, registers.get(R0), registers.get(R1), registers.get(R2),
			registers.get(R3), registers.get(R4));
		backtrace.addOpPCPair(pc - 2, opcode);

		pc &= ~1;

		pipeline[0] = pipeline[1];
		pc += 2;
		pipeline[1] = memory->Read(Half, pc, SEQ);

		Operation = ThumbOperation(opcode);
	} else {
		spdlog::get("std")->debug(
			"PC:{:X} - Op:{:X} - R0:{:X} - R1:{:X} - R2:{:X} - R3:{:X} - R4:{:X}",
			pc - 4, opcode, registers.get(R0), registers.get(R1), registers.get(R2),
			registers.get(R3), registers.get(R4));
		backtrace.addOpPCPair(pc - 4, opcode);

		pc &= ~3;

		pipeline[0] = pipeline[1];
		pc += 4;
		pipeline[1] = memory->Read(Word, pc, SEQ);
	}

	if (!HandleInterruptRequests()) {
		Operation();
	}

	spdlog::get("std")->trace("************************");
}

StateView CPU::ViewState()
{
	RegisterView regView;
	for (int i = 0; i < 16; i++) {
		regView[i] = registers.view((Register)i);
	}

	StateView stateView{ regView, backtrace };
	return stateView;
}

bool CPU::HandleInterruptRequests()
{
	auto& cpsr = registers.get(CPSR);
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

void CPU::PipelineFlush()
{
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

ParamList CPU::ParseParams(OpCode opcode, ParamSegments paramSegs)
{
	ParamList params;
	for (auto it = paramSegs.rbegin(); it != paramSegs.rend(); ++it) {
		auto param = BIT_RANGE(opcode, it->second, it->first);
		params.push_back(param);
	}
	return params;
}

} // namespace ARM7TDMI