#pragma once

#include "int.hpp"
#include "apu/apu_io_registers.hpp"
#include "common/circular_queue.hpp"

class APU : public APUIORegisters {

public:
	
	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality&) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality&) override;

private:
	CircularQueue<U32, 8> fifo[2];

};
