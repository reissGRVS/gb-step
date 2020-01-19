#pragma once
#include "int.hpp"
#include "memory/read_write_interface.hpp"

class LCDIORegisters : public ReadWriteInterface {

public:
	std::string Name() override { return "LCD_IO"; };
	static const U32 LCD_IO_START = 0x04000000, LCD_IO_END = 0x04000058;
	static const U32 LCD_IO_SIZE = LCD_IO_END - LCD_IO_START;

protected:
	std::array<U8, LCD_IO_SIZE> registers{};
};
