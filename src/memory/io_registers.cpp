#include "memory/io_registers.hpp"

IORegisters::IORegisters(std::shared_ptr<LCDIORegisters> lcd)
	: lcd(lcd)
{
}

std::shared_ptr<ReadWriteInterface> IORegisters::GetRegisterSet(U32 address)
{
	if (IN_RANGE(address, LCDIORegisters::LCD_IO_START, LCDIORegisters::LCD_IO_END)) {
		return lcd;
		//TODO: Add other range checks here
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