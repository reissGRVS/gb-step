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

U32 IORegisters::Read(AccessSize size,
	U32 address,
	Sequentiality seq)
{
	if (size == Word) {
		auto lowerHalf = Read(Half, address, seq);
		auto upperHalf = Read(Half, address + 2, seq);
		return lowerHalf + (upperHalf << 16);
	}

	spdlog::get("std")->trace("IO Read @ {:X}", address);
	auto regSet = GetRegisterSet(address);
	if (regSet.get() != nullptr) {
		return regSet->Read(size, address, seq);
	} else {
		U32 actualIndex = address - IOREG_START;
		auto value = ReadToSize(backup, actualIndex, size);
		spdlog::get("std")->info("Unhandled IO Read {:X} @ {:X}", value, address);
		return value;
	}
}

void IORegisters::Write(AccessSize size,
	U32 address,
	U32 value,
	Sequentiality seq)
{

	if (size == Word) {
		Write(Half, address, value & Half, seq);
		Write(Half, address + 2, value >> 16, seq);
		return;
	}

	spdlog::get("std")->trace("IO Write {:X} @ {:X}", value, address);
	auto regSet = GetRegisterSet(address);
	if (regSet.get() != nullptr) {
		regSet->Write(size, address, value, seq);
	} else {
		U32 actualIndex = address - IOREG_START;
		spdlog::get("std")->info("Unhandled IO Write {:X} @ {:X}", value, address);
		WriteToSize(backup, actualIndex, value, size);
	}
}