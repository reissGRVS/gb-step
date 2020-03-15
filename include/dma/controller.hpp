#pragma once

#include "dma/channel.hpp"
#include "dma/dma_io_registers.hpp"
#include "memory/memory.hpp"
#include <array>

namespace DMA {
class Controller : public DMAIORegisters {
public:
	Controller(std::shared_ptr<Memory> memory)
		: memory(memory){};

	enum Event { IMMEDIATE,
		VBLANK,
		HBLANK,
		SPECIAL,
		FIFOA,
		FIFOB };

	void Execute();
	bool IsActive();
	void CntHUpdateCallback(U8 id, U16 value);
	void EventCallback(Event event, bool start);

	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality&) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality&) override;

private:
	std::shared_ptr<Memory> memory;
	bool controllerActive = false;
	std::unique_ptr<Channel> channels[4] = { std::make_unique<Channel>(0, memory),
		std::make_unique<Channel>(1, memory),
		std::make_unique<Channel>(2, memory),
		std::make_unique<Channel>(3, memory) };
};
} // namespace DMA
