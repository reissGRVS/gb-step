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
  CPU(std::shared_ptr<Memory> memory_) : memory(memory_){};
  void Execute();
  RegisterSet registers;

 private:
  std::shared_ptr<Memory> memory;
  std::array<OpCode, 2> pipeline;

  void PipelineFlush();

  // ARM Ops and Data
  ParamList ParseParams(OpCode opcode, ParamSegments paramSegs);

  void Shift(std::uint32_t& value,
             const std::uint32_t amount,
             const std::uint32_t& shiftType,
             std::uint8_t& carryOut);

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

  const ParamSegments DataProcessingSegments
      //  I       Opcode  S       Rn      Rd      Op2
      = {{25, 25}, {24, 21}, {20, 20}, {19, 16}, {15, 12}, {11, 0}};
  void ArmDataProcessing(ParamList params);

  const ParamSegments MultiplySegments
      //  A       S       Rd      Rn      Rs     Rm
      = {{21, 21}, {20, 20}, {19, 16}, {15, 12}, {11, 8}, {3, 0}};
  void ArmMultiply(ParamList params);

  const ParamSegments MultiplyLongSegments
      //  U       A       S       RdHi    RdLo    Rn     Rm
      = {{22, 22}, {21, 21}, {20, 20}, {19, 16}, {15, 12}, {11, 8}, {3, 0}};
  void ArmMultiplyLong(ParamList params);

  const ParamSegments SingleDataSwapSegments
      //  B       Rn      Rd      Rm
      = {{22, 22}, {19, 16}, {15, 12}, {3, 0}};
  void ArmSingleDataSwap(ParamList params);

  const ParamSegments BranchAndExchangeSegments
      //  Rn
      = {{3, 0}};
  void ArmBranchAndExchange(ParamList params);

  const ParamSegments HalfwordDTRegOffsetSegments
      //  P       U       W       L       Rn      Rd      S     H     Rm
      = {{24, 24}, {23, 23}, {21, 21}, {20, 20}, {19, 16},
         {15, 12}, {6, 6},   {5, 5},   {3, 0}};
  void ArmHalfwordDTRegOffset(ParamList params);

  const ParamSegments HalfwordDTImmOffsetSegments
      //  P       U       W       L       Rn      Rd      Offset S     H Offset
      = {{24, 24}, {23, 23}, {21, 21}, {20, 20}, {19, 16},
         {15, 12}, {11, 8},  {6, 6},   {5, 5},   {3, 0}};
  void ArmHalfwordDTImmOffset(ParamList params);

  const ParamSegments SingleDataTransferSegments
      //  I       P       U       B       W       L       Rn      Rd      Offset
      = {{25, 25}, {24, 24}, {23, 23}, {22, 22}, {21, 21},
         {20, 20}, {19, 16}, {15, 12}, {11, 0}};
  void ArmSingleDataTransfer(ParamList params);

  // No Params
  void ArmUndefined(ParamList params);

  const ParamSegments BlockDataTransferSegments
      //  P       U       S       W       L       Rn      RegList
      = {{24, 24}, {23, 23}, {22, 22}, {21, 21}, {20, 20}, {19, 16}, {15, 0}};
  void ArmBlockDataTransfer(ParamList params);

  const ParamSegments BranchSegments
      //  L       Offset
      = {{24, 24}, {23, 0}};
  void ArmBranch(ParamList params);

  // No Params
  void ArmSWI(ParamList params);
};
}  // namespace ARM7TDMI