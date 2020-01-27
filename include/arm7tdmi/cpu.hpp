#pragma once

#include "int.hpp"
#include <array>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "arm7tdmi/ir_io_registers.hpp"
#include "arm7tdmi/op_cache.hpp"
#include "arm7tdmi/types.hpp"
#include "memory/read_write_interface.hpp"
#include "memory/memory.hpp"
#include "opbacktrace.hpp"
#include "registers.hpp"
#include "stateview.hpp"
#include "system_clock.hpp"

namespace ARM7TDMI {

class CPU : public IRIORegisters {
public:
	CPU(std::shared_ptr<SystemClock> clock, std::shared_ptr<Memory> memory)
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

	OpCache armOps;
	OpCache thumbOps;

	bool slow = false;
	std::shared_ptr<SystemClock> clock;
	std::shared_ptr<Memory> memory;
	std::array<OpCode, 2> pipeline;

	void PipelineFlush();
	bool HandleInterruptRequests();

	void ParseParams(const OpCode& opcode, const ParamSegments& paramSegs);

	void Shift(U32& value,
		const U32 amount,
		const U32& shiftType,
		bool& carryOut,
		bool regProvidedAmount);

	
	// Thumb Operations
	Op ThumbOperation(OpCode opcode);

	ParamList params;
	Op ThumbMoveShiftedReg_P();
	Op ThumbAddSubtract_P();
	Op ThumbMoveCompAddSubImm_P();
	Op ThumbALUOps_P();
	Op ThumbHiRegOps_P();
	Op ThumbPCRelativeLoad_P();
	void ThumbPCRelativeLoad(U16 Word8 , U16 Rd);
	Op ThumbLSRegOff_P();
	Op ThumbLSSignExt_P();
	Op ThumbLSImmOff_P();
	Op ThumbLSHalf_P();
	Op ThumbSPRelativeLS_P();
	Op ThumbLoadAddress_P();
	Op ThumbOffsetSP_P();
	Op ThumbPushPopReg_P();
	Op ThumbMultipleLS_P();
	Op ThumbCondBranch_P();
	void ThumbCondBranch(U16 SOffset8, U16 Cond);
	void ThumbSWI();
	Op ThumbUncondBranch_P();
	void ThumbUncondBranch(U16 Offset11);
	Op ThumbLongBranchLink_P();
	void ThumbLongBranchLink(U16 Offset,  U16 H);

	Op ArmOperation(OpCode opcode);
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
	Op ArmDataProcessing_P();
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
	Op ArmMultiply_P();
	void ArmMultiply(
		U32 A,
		U32 S,
		U32 Rd,
		U32 Rn,
		U32 Rs,
		U32 Rm);

	Op ArmMultiplyLong_P();
	void ArmMultiplyLong(
		U32 U,
		U32 A,
		U32 S,
		U32 RdHi,
		U32 RdLo,
		U32 Rs,
		U32 Rm);

	Op ArmSingleDataSwap_P();
	void ArmSingleDataSwap(
		U32 B,
		U32 Rn,
		U32 Rd,
		U32 Rm);

	Op ArmBranchAndExchange_P();
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
	Op ArmHalfwordDTRegOffset_P();
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

	Op ArmHalfwordDTImmOffset_P();
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

	Op ArmSingleDataTransfer_P();
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
	Op ArmUndefined_P();
	void ArmUndefined();

	Op ArmBlockDataTransfer_P();
	void ArmBlockDataTransfer(
		U32 P,
		U32 U,
		U32 S,
		U32 W,
		U32 L,
		U32 Rn,
		U32 RegList);

	Op ArmBranch_P();
	void ArmBranch(U32 L, U32 Offset);
	// No Params
	Op ArmSWI_P();
	void ArmSWI();
};
} // namespace ARM7TDMI