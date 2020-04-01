#pragma once

#include "apu/apu_io_registers.hpp"
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
		std::shared_ptr<IRIORegisters> ir,
		std::shared_ptr<APUIORegisters> apu);

	std::string Name() override { return "IO_BASE"; };
	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality& seq) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality& seq) override;

	virtual ~IORegisters() = default;

private:
	std::shared_ptr<ReadWriteInterface> GetRegisterSet(U32 address);

	std::shared_ptr<TimersIORegisters> timers;
	std::shared_ptr<DMAIORegisters> dma;
	std::shared_ptr<LCDIORegisters> lcd;
	std::shared_ptr<IRIORegisters> ir;
	std::shared_ptr<APUIORegisters> apu;

	std::array<U8, IOREG_SIZE>
		backup {};
};