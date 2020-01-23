#pragma once

#include "int.hpp"
#include <array>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "arm7tdmi/ir_io_registers.hpp"
#include "arm7tdmi/types.hpp"
#include "memory/read_write_interface.hpp"
#include "opbacktrace.hpp"
#include "registers.hpp"
#include "stateview.hpp"
#include "system_clock.hpp"

namespace ARM7TDMI {

class CPU : public IRIORegisters {
public:
	CPU(std::shared_ptr<SystemClock> clock, std::shared_ptr<ReadWriteInterface> memory)
		: clock(clock)
		, memory(memory)
	{
	}

	void Reset();

	void Execute();

	//For Interrupt IO Registers
	U32 Read(AccessSize size,
		U32 address,
		Sequentiality) override;
	void Write(AccessSize size,
		U32 address,
		U32 value,
		Sequentiality) override;

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

	void ParseParams(OpCode opcode, const ParamSegments& paramSegs);

	void Shift(U32& value,
		const U32 amount,
		const U32& shiftType,
		bool& carryOut,
		bool regProvidedAmount);

	// Thumb Operations
	std::function<void()> ThumbOperation(OpCode opcode);

	ParamList params;
	std::function<void()> ThumbMoveShiftedReg_P();
	std::function<void()> ThumbAddSubtract_P();
	std::function<void()> ThumbMoveCompAddSubImm_P();
	std::function<void()> ThumbALUOps_P();
	std::function<void()> ThumbHiRegOps_P();
	std::function<void()> ThumbPCRelativeLoad_P();
	std::function<void()> ThumbLSRegOff_P();
	std::function<void()> ThumbLSSignExt_P();
	std::function<void()> ThumbLSImmOff_P();
	std::function<void()> ThumbLSHalf_P();
	std::function<void()> ThumbSPRelativeLS_P();
	std::function<void()> ThumbLoadAddress_P();
	std::function<void()> ThumbOffsetSP_P();
	std::function<void()> ThumbPushPopReg_P();
	std::function<void()> ThumbMultipleLS_P();
	std::function<void()> ThumbCondBranch_P();
	void ThumbCondBranch(U16 SOffset8, U16 Cond);
	void ThumbSWI();
	std::function<void()> ThumbUncondBranch_P();
	void ThumbUncondBranch(U16 Offset11);
	std::function<void()> ThumbLongBranchLink_P();
	void ThumbLongBranchLink(U16 Offset,  U16 H);

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
	std::function<void()> ArmDataProcessing_P();
	void ArmDataProcessing(
		U32 I,
		U32 OpCode,
		U32 S,
		U32 Rn,
		U32 Rd,
		U32 Op2);
	void DataProcessing(U32 I, U32 OpCode, U32 S, U32 Rn, U32 Rd, U32 Op2, bool Adr);
	
	void ArmMRS(bool Ps, U8 Rd);
	void ArmMSR(bool I, bool Pd, bool flagsOnly, U16 source);

	void ICyclesMultiply(const U32& mulop);
	std::function<void()> ArmMultiply_P();
	void ArmMultiply(
		U32 A,
		U32 S,
		U32 Rd,
		U32 Rn,
		U32 Rs,
		U32 Rm);

	std::function<void()> ArmMultiplyLong_P();
	void ArmMultiplyLong(
		U32 U,
		U32 A,
		U32 S,
		U32 RdHi,
		U32 RdLo,
		U32 Rs,
		U32 Rm);

	std::function<void()> ArmSingleDataSwap_P();
	void ArmSingleDataSwap(
		U32 B,
		U32 Rn,
		U32 Rd,
		U32 Rm);

	std::function<void()> ArmBranchAndExchange_P();
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
	std::function<void()> ArmHalfwordDTRegOffset_P();
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

	std::function<void()> ArmHalfwordDTImmOffset_P();
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

	std::function<void()> ArmSingleDataTransfer_P();
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
	std::function<void()> ArmUndefined_P();
	void ArmUndefined();

	std::function<void()> ArmBlockDataTransfer_P();
	void ArmBlockDataTransfer(
		U32 P,
		U32 U,
		U32 S,
		U32 W,
		U32 L,
		U32 Rn,
		U32 RegList);

	std::function<void()> ArmBranch_P();
	void ArmBranch(U32 L, U32 Offset);
	// No Params
	std::function<void()> ArmSWI_P();
	void ArmSWI();
};
} // namespace ARM7TDMI