#pragma once
#include "memory/memory.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace DMA {
class Channel {
 public:
  Channel(std::int_fast8_t id, std::shared_ptr<Memory> memory_);

  void ReloadDAD();
  void ReloadSAD();
  void ReloadWordCount();
  void CalculateTransferSteps();
  void UpdateDetails(std::uint16_t value);

  void DoTransferStep();

  const std::int_fast8_t ID;
  const std::uint32_t SAD;
  const std::uint32_t DAD;
  const std::uint32_t CNT_L;
  const std::uint32_t CNT_H;

  bool active = false;

  std::uint16_t dmaCnt;
  std::uint16_t enable = 0;
  std::uint16_t repeat;
  std::uint16_t startTiming;
  std::uint16_t irqAtEnd;
  std::uint16_t transferType;
  std::uint16_t transferSize;
  std::uint16_t srcStep;
  std::uint16_t destStep;
  std::uint16_t destAddrCtl;
  std::uint16_t srcAddrCtl;

 private:
  std::shared_ptr<Memory> memory;

  std::uint32_t source;
  std::uint32_t dest;
  std::uint16_t wordCount;
};
}  // namespace DMA