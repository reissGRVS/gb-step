#pragma once

#include "int.hpp"
#include <array>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "arm7tdmi/types.hpp"
#include "memory/read_write_interface.hpp"
#include "opbacktrace.hpp"
#include "registers.hpp"
#include "stateview.hpp"
#include "system_clock.hpp"

namespace ARM7TDMI {
class CPU {
public:
	CPU(std::shared_ptr<SystemClock> clock, std::shared_ptr<ReadWriteInterface> memory)
		: clock(clock)
		, memory(memory)
	{
		// Skip BIOS

		registers.get(ModeBank::SVC, Register::R13) = 0x03007FE0;
		registers.get(ModeBank::IRQ, Register::R13) = 0x03007FA0;
		registers.get(Register::R13) = 0x03007F00;
		registers.switchMode(SRFlag::ModeBits::USR);
		registers.get(Register::R15) = 0x08000000;

		PipelineFlush();
	};

	void Execute();

	StateView ViewState();
	RegisterSet registers;
	OpBacktrace backtrace;

private:
	bool slow = false;
	std::shared_ptr<SystemClock> clock;
	std::shared_ptr<ReadWriteInterface> memory;
	std::array<OpCode, 2> pipeline;

	void PipelineFlush();
	bool HandleInterruptRequests();

	ParamList ParseParams(OpCode opcode, ParamSegments paramSegs);

	void Shift(U32& value,
		const U32 amount,
		const U32& shiftType,
		U8& carryOut,
		bool regProvidedAmount);

	// Thumb Operations
	std::function<void()> ThumbOperation(OpCode opcode);

	void ThumbMoveShiftedReg_P(ParamList params);
	void ThumbAddSubtract_P(ParamList params);
	void ThumbMoveCompAddSubImm_P(ParamList params);
	void ThumbALUOps_P(ParamList params);
	void ThumbHiRegOps_P(ParamList params);
	void ThumbPCRelativeLoad_P(ParamList params);
	void ThumbLSRegOff_P(ParamList params);
	void ThumbLSSignExt_P(ParamList params);
	void ThumbLSImmOff_P(ParamList params);
	void ThumbLSHalf_P(ParamList params);
	void ThumbSPRelativeLS_P(ParamList params);
	void ThumbLoadAddress_P(ParamList params);
	void ThumbOffsetSP_P(ParamList params);
	void ThumbPushPopReg_P(ParamList params);
	void ThumbMultipleLS_P(ParamList params);
	void ThumbCondBranch_P(ParamList params);
	void ThumbSWI_P(ParamList params);
	void ThumbUncondBranch_P(ParamList params);
	void ThumbLongBranchLink_P(ParamList params);

	std::function<void()> ArmOperation(OpCode opcode);
	// ARM Operations

	enum DPOps {
		AND,
		EOR,
		SUB,
		RSB,
		ADD,
		ADC,
		SBC,
		RSC,
		TST,
		TEQ,
		CMP,
		CMN,
		ORR,
		MOV,
		BIC,
		MVN
	};
	void ArmDataProcessing_P(ParamList params);
	void ArmDataProcessing(
		U32 I,
		U32 OpCode,
		U32 S,
		U32 Rn,
		U32 Rd,
		U32 Op2);
	void ArmMRS(bool Ps, U8 Rd);
	void ArmMSR(bool I, bool Pd, bool flagsOnly, U16 source);

	void ICyclesMultiply(const U32& mulop);
	void ArmMultiply_P(ParamList params);
	void ArmMultiply(
		U32 A,
		U32 S,
		U32 Rd,
		U32 Rn,
		U32 Rs,
		U32 Rm);

	void ArmMultiplyLong_P(ParamList params);
	void ArmMultiplyLong(
		U32 U,
		U32 A,
		U32 S,
		U32 RdHi,
		U32 RdLo,
		U32 Rs,
		U32 Rm);

	void ArmSingleDataSwap_P(ParamList params);
	void ArmSingleDataSwap(
		U32 B,
		U32 Rn,
		U32 Rd,
		U32 Rm);

	void ArmBranchAndExchange_P(ParamList params);
	void ArmBranchAndExchange(U32 Rn);

	void ArmHalfwordDT(
		U32 P,
		U32 U,
		U32 W,
		U32 L,
		U32 Rn,
		U32 Rd,
		U32 S,
		U32 H,
		U32 Offset);
	void ArmHalfwordDTRegOffset_P(ParamList params);
	void ArmHalfwordDTRegOffset(
		U32 P,
		U32 U,
		U32 W,
		U32 L,
		U32 Rn,
		U32 Rd,
		U32 S,
		U32 H,
		U32 Rm);

	void ArmHalfwordDTImmOffset_P(ParamList params);
	void ArmHalfwordDTImmOffset(
		U32 P,
		U32 U,
		U32 W,
		U32 L,
		U32 Rn,
		U32 Rd,
		U32 OffsetHi,
		U32 S,
		U32 H,
		U32 OffsetLo);

	void ArmSingleDataTransfer_P(ParamList params);
	void ArmSingleDataTransfer(
		U32 I,
		U32 P,
		U32 U,
		U32 B,
		U32 W,
		U32 L,
		U32 Rn,
		U32 Rd,
		U32 Offset);
	// No Params
	void ArmUndefined_P(ParamList params);
	void ArmUndefined();

	void ArmBlockDataTransfer_P(ParamList params);
	void ArmBlockDataTransfer(
		U32 P,
		U32 U,
		U32 S,
		U32 W,
		U32 L,
		U32 Rn,
		U32 RegList);

	void ArmBranch_P(ParamList params);
	void ArmBranch(U32 L, U32 Offset);
	// No Params
	void ArmSWI_P(ParamList params);
	void ArmSWI();
};
} // namespace ARM7TDMI