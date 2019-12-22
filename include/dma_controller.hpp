#pragma once

#include <array>
#include "dma_channel.hpp"
#include "memory.hpp"

class DMAController {
 public:
  DMAController(std::shared_ptr<Memory> memory) : memory(memory){};

  enum Event { IMMEDIATE, VBLANK, HBLANK, SPECIAL };

  void Execute() {
	for (const auto& c : channels) {
	  if (c->active) {
		c->DoTransferStep();
		return;
	  }
	}
  }

  bool IsActive() {
	for (const auto& c : channels) {
	  if (c->active) {
		return true;
	  }
	}
	return false;
  }

  void CntHUpdateCallback(std::uint_fast8_t id, std::uint16_t value) {
	channels[id]->UpdateDetails(value);

	if (channels[id]->enable && channels[id]->startTiming == (uint)IMMEDIATE) {
	  channels[id]->active = true;
	}
  };

  void EventCallback(Event event, bool start) {
	for (const auto& c : channels) {
	  if (c->enable && c->startTiming == (std::uint16_t)event) {
		if (start) {
		  c->active = true;
		} else {
		  c->active = false;
		}
	  }
	}
  };

 private:
  std::shared_ptr<Memory> memory;

  std::unique_ptr<DMAChannel> channels[4] = {
      std::make_unique<DMAChannel>(0, memory),
      std::make_unique<DMAChannel>(1, memory),
      std::make_unique<DMAChannel>(2, memory),
      std::make_unique<DMAChannel>(3, memory)};
};