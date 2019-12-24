#pragma once

#include "arm7tdmi/ir_io_registers.hpp"
#include "dma/dma_io_registers.hpp"
#include "int.hpp"
#include "memory/read_write_interface.hpp"
#include "memory/regions.hpp"
#include "ppu/lcd_io_registers.hpp"
#include "timers/timers_io_registers.hpp"
#include "utils.hpp"
#include <memory>

class IORegisters : public ReadWriteInterface {
public:
	IORegisters(std::shared_ptr<TimersIORegisters> timers,
		std::shared_ptr<DMAIORegisters> dma,
		std::shared_ptr<LCDIORegisters> lcd,
		std::shared_ptr<IRIORegisters> ir);

	U32 Read(AccessSize size,
		U32 address,
		Sequentiality seq) override;
	void Write(AccessSize size,
		U32 address,
		U32 value,
		Sequentiality seq) override;

private:
	std::shared_ptr<ReadWriteInterface> GetRegisterSet(U32 address);

	std::shared_ptr<TimersIORegisters> timers;
	std::shared_ptr<DMAIORegisters> dma;
	std::shared_ptr<LCDIORegisters> lcd;
	std::shared_ptr<IRIORegisters> ir;

	std::array<U8, IOREG_SIZE>
		backup{};
};