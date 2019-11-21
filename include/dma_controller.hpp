#pragma once

#include <array>
#include "dma_channel.hpp"
#include "memory.hpp"

class DMAController {
 public:
  DMAController(std::shared_ptr<Memory> memory) : memory(memory){};

  enum Event { IMMEDIATE, VBLANK, HBLANK, SPECIAL };

  // TODO: Fix so that if one is running another one doesnt stop running (if
  // higher priority), will be relevant when doing this properly and getting
  // HBlank interrupts mid transfer
  void CntHUpdateCallback(std::uint_fast8_t id) {
	channels[id].updateDetails();

	if (channels[id].enable && channels[id].startTiming == (uint)IMMEDIATE) {
	  channels[id].doTransfer();
	}
  };

  void EventCallback(Event event) {
	for (DMAChannel c : channels) {
	  if (c.enable && c.startTiming == (std::uint16_t)event) {
		c.doTransfer();
	  }
	}
  };

 private:
  std::shared_ptr<Memory> memory;

  std::int_fast8_t activeChannel = -1;
  const std::int_fast8_t NO_ACTIVE_CHANNEL = -1;
  std::array<DMAChannel, 4> channels = {
      DMAChannel(0, memory),
      DMAChannel(1, memory),
      DMAChannel(2, memory),
      DMAChannel(3, memory),
  };
};