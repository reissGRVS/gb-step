#include "dma/controller.hpp"

using namespace DMA;

void Controller::Execute() {
  for (const auto& c : channels) {
	if (c->active) {
	  c->DoTransferStep();
	  return;
	}
  }
}

bool Controller::IsActive() {
  for (const auto& c : channels) {
	if (c->active) {
	  return true;
	}
  }
  return false;
}

void Controller::CntHUpdateCallback(std::uint_fast8_t id, std::uint16_t value) {
  channels[id]->UpdateDetails(value);

  if (channels[id]->enable && channels[id]->startTiming == (uint)IMMEDIATE) {
	channels[id]->active = true;
  }
}

void Controller::EventCallback(Event event, bool start) {
  for (const auto& c : channels) {
	if (c->enable && c->startTiming == (std::uint16_t)event) {
	  if (start) {
		c->active = true;
	  } else {
		c->active = false;
	  }
	}
  }
}
