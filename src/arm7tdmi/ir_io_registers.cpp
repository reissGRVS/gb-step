#include "arm7tdmi/cpu.hpp"
#include "spdlog/spdlog.h"

void IRIORegisters::InterruptUpdate() {
  auto ie = ReadToSize(ioregisters, IR_IE, Half);
  auto irf = ReadToSize(ioregisters, IR_IF, Half);
  auto ime = ReadToSize(ioregisters, IR_IME, Half);
  auto interruptMatch = ie & irf;
  if (interruptMatch) {
    halt = false;
  }
  interruptReady = ime && interruptMatch;
}

void IRIORegisters::WaitstateControlUpdate() {

  const std::array<uint8_t, 4> NSEQ = {4, 3, 2, 8};
  const std::array<uint8_t, 2> WS0_SEQ = {2, 1};
  const std::array<uint8_t, 2> WS1_SEQ = {4, 1};
  const std::array<uint8_t, 2> WS2_SEQ = {8, 1};

  auto waitCnt = ReadToSize(ioregisters, IR_WAITCNT, Half);

  waitstateCounts[WS0].nseq = NSEQ[BIT_RANGE(waitCnt, 2, 3)] + 1;
  waitstateCounts[WS0].seq = WS0_SEQ[BIT_RANGE(waitCnt, 4, 4)] + 1;

  waitstateCounts[WS1].nseq = NSEQ[BIT_RANGE(waitCnt, 5, 6)] + 1;
  waitstateCounts[WS1].seq = WS1_SEQ[BIT_RANGE(waitCnt, 7, 7)] + 1;

  waitstateCounts[WS2].nseq = NSEQ[BIT_RANGE(waitCnt, 8, 9)] + 1;
  waitstateCounts[WS2].seq = WS2_SEQ[BIT_RANGE(waitCnt, 10, 10)] + 1;

  waitstateCounts[SRAM].nseq = NSEQ[BIT_RANGE(waitCnt, 0, 1)] + 1;
  waitstateCounts[SRAM].seq = NSEQ[BIT_RANGE(waitCnt, 0, 1)] + 1;
}

namespace ARM7TDMI {
U32 CPU::Read(const AccessSize& size, U32 address, const Sequentiality&) {
  U32 actualIndex = address - IR_IO_START;
  return ReadToSize(ioregisters, actualIndex, size);
}

void CPU::Write(const AccessSize& size, U32 address, U32 value, const Sequentiality& seq) {
  if (size == Word) {

    exit(-1);
  }

  U32 actualIndex = address - IR_IO_START;
  if (seq != FREE) {
    if (address == IF || address == IF + 1) {
      value &= size;
      auto curIF = ReadToSize(ioregisters, actualIndex, size);

      value = curIF & ~value;

    } else if (address == HALTCNT) {
      halt = true;
    }
  }

  WriteToSize(ioregisters, actualIndex, value, size);

  if (address == WAITCNT || address == WAITCNT + 1) {
    WaitstateControlUpdate();
  }
  if (IN_RANGE(address, IE, IME + 2)) {
    InterruptUpdate();
  }

  // Internal Memory Control
  if (BIT_RANGE(address, 0, 11) == 0x800) {

    exit(-1);
  }
}
}
