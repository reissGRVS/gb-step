#include "dma/controller.hpp"

using namespace DMA;

void Controller::Execute()
{
	for (const auto& c : channels) {
		if (c->active) {
			if (c->startTiming == (U16)Event::SPECIAL)
				c->DoSoundTransfer();
			else
				c->DoTransferStep();

			return;
		}
	}
	controllerActive = false;
	return;
}

bool Controller::IsActive()
{
	if (!controllerActive) {
		return false;
	}

	for (const auto& c : channels) {
		if (c->active) {
			return true;
		}
	}
	controllerActive = false;
	return false;
}

void Controller::CntHUpdateCallback(U8 id, U16 value)
{
	channels[id]->UpdateDetails(value);

	if (channels[id]->enable && channels[id]->startTiming == (U16)IMMEDIATE) {
		channels[id]->active = true;
		controllerActive = true;
	}
}

void Controller::EventCallback(Event event, bool start)
{
	if (event == Event::FIFOA || event == Event::FIFOB) {
		for (auto channel_index = 1; channel_index <= 2; channel_index++) {
			const auto& channel = channels[channel_index];

			auto dest = FIFO_A;
			if (event == Event::FIFOB)
				dest = FIFO_B;

			if (channel->enable && channel->startTiming == (U16)Event::SPECIAL && channel->dest == dest) {
				if (start) {
					channel->active = true;
					controllerActive = true;
				} else {
					channel->active = false;
				}
			}
		}
	} else {
		for (const auto& c : channels) {
			if (c->enable && c->startTiming == (U16)event) {
				if (start) {
					c->active = true;
					controllerActive = true;
				} else {
					c->active = false;
				}
			}
		}
	}
}
