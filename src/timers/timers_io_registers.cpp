#include "timers/timers.hpp"
#include "memory/regions.hpp"
#include "spdlog/spdlog.h"

U32 Timers::Read(AccessSize size, U32 address, Sequentiality seq) {
  if (size == Byte) {
    auto half = Read(Half, address & ~1, seq);
    if (address & 1) {
      return (half >> 8);
    } else {
      return half & Byte;
    }
  }

  switch (address) {
  case TM0CNT_L:
    return timers[0].counter;
  case TM0CNT_H:
    return timers[0].cntHData;
  case TM1CNT_L:
    return timers[1].counter;
  case TM1CNT_H:
    return timers[1].cntHData;
  case TM2CNT_L:
    return timers[2].counter;
  case TM2CNT_H:
    return timers[2].cntHData;
  case TM3CNT_L:
    return timers[3].counter;
  case TM3CNT_H:
    return timers[3].cntHData;
  default: {
    spdlog::get("std")->error("Invalid Timer Read");
    exit(-1);
  }
  }
}

void Timers::Write(AccessSize size, U32 address, U32 value, Sequentiality) {
  if (size == Byte) {
    spdlog::get("std")->error("Byte Timer Write - Needs implemented?");
    exit(-1);
  }

  switch (address) {
  case TM0CNT_L:
    SetReloadValue(0, value);
    break;
  case TM0CNT_H:
    TimerCntHUpdate(0, value);
    break;
  case TM1CNT_L:
    SetReloadValue(1, value);
    break;
  case TM1CNT_H:
    TimerCntHUpdate(1, value);
    break;
  case TM2CNT_L:
    SetReloadValue(2, value);
    break;
  case TM2CNT_H:
    TimerCntHUpdate(2, value);
    break;
  case TM3CNT_L:
    SetReloadValue(3, value);
    break;
  case TM3CNT_H:
    TimerCntHUpdate(3, value);
    break;
  default:
    break;
  }
}