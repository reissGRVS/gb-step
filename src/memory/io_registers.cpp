#include "memory/io_registers.hpp"

IORegisters::IORegisters(std::shared_ptr<TimersIORegisters> timers,
	std::shared_ptr<DMAIORegisters> dma,
	std::shared_ptr<LCDIORegisters> lcd,
	std::shared_ptr<IRIORegisters> ir)
	: timers(timers)
	, dma(dma)
	, lcd(lcd)
	, ir(ir)
{
}

std::shared_ptr<ReadWriteInterface> IORegisters::GetRegisterSet(U32 address)
{
	if (IN_RANGE(address, LCDIORegisters::LCD_IO_START, LCDIORegisters::LCD_IO_END)) {
		return lcd;
	} else if (IN_RANGE(address, DMAIORegisters::DMA_IO_START, DMAIORegisters::DMA_IO_END)) {
		return dma;
	} else if (IN_RANGE(address, IRIORegisters::IR_IO_START, IRIORegisters::IR_IO_END)) {
		return ir;
	} else if (IN_RANGE(address, TimersIORegisters::TIMER_IO_START, TimersIORegisters::TIMER_IO_END)) {
		return timers;
	}

	return {};
}

U32 IORegisters::Read(AccessSize size,
	U32 address,
	Sequentiality seq)
{
	auto regSet = GetRegisterSet(address);
	if (regSet.get() != nullptr) {
		return regSet->Read(size, address, seq);
	} else {
		U32 actualIndex = address - IOREG_START;
		return ReadToSize(&backup[actualIndex], size);
	}
}

void IORegisters::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality seq)
{
	auto regSet = GetRegisterSet(address);
	if (regSet.get() != nullptr) {
		regSet->Read(size, address, seq);
	} else {
		U32 actualIndex = address - IOREG_START;
		WriteToSize(&backup[actualIndex], value, size);
	}
}