#include "memory/io_registers.hpp"
#include "spdlog/spdlog.h"

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

U32 IORegisters::Read(const AccessSize& size,
	U32 address,
	const Sequentiality& seq)
{
	if (size == Word) {
		auto lowerHalf = Read(Half, address, seq);
		auto upperHalf = Read(Half, address + 2, seq);
		return lowerHalf + (upperHalf << 16);
	}

	
	auto regSet = GetRegisterSet(address);
	if (regSet.get() != nullptr) {
		return regSet->Read(size, address, seq);
	} else {
		U32 actualIndex = address - IOREG_START;
		auto value = ReadToSize(backup, actualIndex, size);
		
		return value;
	}
}

void IORegisters::Write(const AccessSize& size,
	U32 address,
	U32 value,
	const Sequentiality& seq)
{

	if (size == Word) {
		Write(Half, address, value & Half, seq);
		Write(Half, address + 2, value >> 16, seq);
		return;
	}

	
	auto regSet = GetRegisterSet(address);
	if (regSet.get() != nullptr) {
		regSet->Write(size, address, value, seq);
	} else {
		U32 actualIndex = address - IOREG_START;
		
		WriteToSize(backup, actualIndex, value, size);
	}
}