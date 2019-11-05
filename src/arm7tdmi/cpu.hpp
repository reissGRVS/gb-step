#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../memory.hpp"
#include "registers.hpp"
#include "types.hpp"

namespace ARM7TDMI {
class CPU {
 public:
  CPU(std::shared_ptr<Memory> memory_) : memory(memory_) {
	registers.get(Register::R15) = 0;
	PipelineFlush();
  };
  std::uint32_t Execute();
  RegisterSet registers;

 private:
  std::shared_ptr<Memory> memory;
  std::array<OpCode, 2> pipeline;

  void PipelineFlush();

  ParamList ParseParams(OpCode opcode, ParamSegments paramSegs);

  void Shift(std::uint32_t& value,
             const std::uint32_t amount,
             const std::uint32_t& shiftType,
             std::uint8_t& carryOut);

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
  void ArmDataProcessing(std::uint32_t I,
                         std::uint32_t OpCode,
                         std::uint32_t S,
                         std::uint32_t Rn,
                         std::uint32_t Rd,
                         std::uint32_t Op2);
  void ArmMRS(bool Ps, std::uint8_t Rd);
  void ArmMSR(bool I, bool Pd, bool flagsOnly, std::uint16_t source);

  void ArmMultiply_P(ParamList params);
  void ArmMultiply(std::uint32_t A,
                   std::uint32_t S,
                   std::uint32_t Rd,
                   std::uint32_t Rn,
                   std::uint32_t Rs,
                   std::uint32_t Rm);

  void ArmMultiplyLong_P(ParamList params);
  void ArmMultiplyLong(std::uint32_t U,
                       std::uint32_t A,
                       std::uint32_t S,
                       std::uint32_t RdHi,
                       std::uint32_t RdLo,
                       std::uint32_t Rs,
                       std::uint32_t Rm);

  void ArmSingleDataSwap_P(ParamList params);
  void ArmSingleDataSwap(std::uint32_t B,
                         std::uint32_t Rn,
                         std::uint32_t Rd,
                         std::uint32_t Rm);

  void ArmBranchAndExchange_P(ParamList params);
  void ArmBranchAndExchange(std::uint32_t Rn);

  void ArmHalfwordDTRegOffset_P(ParamList params);
  void ArmHalfwordDTRegOffset(std::uint32_t P,
                              std::uint32_t U,
                              std::uint32_t W,
                              std::uint32_t L,
                              std::uint32_t Rn,
                              std::uint32_t Rd,
                              std::uint32_t S,
                              std::uint32_t H,
                              std::uint32_t Rm);

  void ArmHalfwordDTImmOffset_P(ParamList params);
  void ArmHalfwordDTImmOffset(std::uint32_t P,
                              std::uint32_t U,
                              std::uint32_t W,
                              std::uint32_t L,
                              std::uint32_t Rn,
                              std::uint32_t Rd,
                              std::uint32_t OffsetHi,
                              std::uint32_t S,
                              std::uint32_t H,
                              std::uint32_t OffsetLo);

  void ArmSingleDataTransfer_P(ParamList params);
  void ArmSingleDataTransfer(std::uint32_t I,
                             std::uint32_t P,
                             std::uint32_t U,
                             std::uint32_t B,
                             std::uint32_t W,
                             std::uint32_t L,
                             std::uint32_t Rn,
                             std::uint32_t Rd,
                             std::uint32_t Offset);
  // No Params
  void ArmUndefined_P(ParamList params);
  void ArmUndefined();

  void ArmBlockDataTransfer_P(ParamList params);
  void ArmBlockDataTransfer(std::uint32_t P,
                            std::uint32_t U,
                            std::uint32_t S,
                            std::uint32_t W,
                            std::uint32_t L,
                            std::uint32_t Rn,
                            std::uint32_t RegList);

  void ArmBranch_P(ParamList params);
  void ArmBranch(std::uint32_t L, std::uint32_t Offset);
  // No Params
  void ArmSWI_P(ParamList params);
  void ArmSWI();
};
}  // namespace ARM7TDMI