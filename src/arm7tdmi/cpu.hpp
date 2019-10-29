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

  std::function<void()> ThumbOperation(OpCode opcode);
  std::function<void()> ArmOperation(OpCode opcode);
  // ARM Operations

  void ArmDataProcessingP(ParamList params);
  void ArmDataProcessing(std::uint32_t I,
                         std::uint32_t OpCode,
                         std::uint32_t S,
                         std::uint32_t Rn,
                         std::uint32_t Rd,
                         std::uint32_t Op2);
  void ArmMRS(bool Ps, std::uint8_t Rd);
  void ArmMSR(bool I, bool Pd, bool flagsOnly, std::uint16_t source);

  void ArmMultiplyP(ParamList params);
  void ArmMultiply(std::uint32_t A,
                   std::uint32_t S,
                   std::uint32_t Rd,
                   std::uint32_t Rn,
                   std::uint32_t Rs,
                   std::uint32_t Rm);

  void ArmMultiplyLongP(ParamList params);
  void ArmMultiplyLong(std::uint32_t U,
                       std::uint32_t A,
                       std::uint32_t S,
                       std::uint32_t RdHi,
                       std::uint32_t RdLo,
                       std::uint32_t Rs,
                       std::uint32_t Rm);

  void ArmSingleDataSwapP(ParamList params);
  void ArmSingleDataSwap(std::uint32_t B,
                         std::uint32_t Rn,
                         std::uint32_t Rd,
                         std::uint32_t Rm);

  void ArmBranchAndExchangeP(ParamList params);
  void ArmBranchAndExchange(std::uint32_t Rn);

  void ArmHalfwordDTRegOffsetP(ParamList params);
  void ArmHalfwordDTRegOffset(std::uint32_t P,
                              std::uint32_t U,
                              std::uint32_t W,
                              std::uint32_t L,
                              std::uint32_t Rn,
                              std::uint32_t Rd,
                              std::uint32_t S,
                              std::uint32_t H,
                              std::uint32_t Rm);

  void ArmHalfwordDTImmOffsetP(ParamList params);
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

  void ArmSingleDataTransferP(ParamList params);
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
  void ArmUndefinedP(ParamList params);
  void ArmUndefined();

  void ArmBlockDataTransferP(ParamList params);
  void ArmBlockDataTransfer(std::uint32_t P,
                            std::uint32_t U,
                            std::uint32_t S,
                            std::uint32_t W,
                            std::uint32_t L,
                            std::uint32_t Rn,
                            std::uint32_t RegList);

  void ArmBranchP(ParamList params);
  void ArmBranch(std::uint32_t L, std::uint32_t Offset);
  // No Params
  void ArmSWIP(ParamList params);
  void ArmSWI();
};
}  // namespace ARM7TDMI