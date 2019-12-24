#pragma once

#include "int.hpp"
#include "memory/read_write_interface.hpp"
#include "memory/regions.hpp"
#include "ppu/lcd_io_registers.hpp"
#include "utils.hpp"
#include <memory>

class IORegisters : public ReadWriteInterface {
public:
	IORegisters(std::shared_ptr<LCDIORegisters> lcd);

	U32 Read(AccessSize size,
		U32 address,
		Sequentiality seq) override;
	void Write(AccessSize size,
		U32 address,
		U32 value,
		Sequentiality seq) override;

private:
	std::shared_ptr<ReadWriteInterface> GetRegisterSet(U32 address);

	std::shared_ptr<LCDIORegisters> lcd;
	//TODO: Add all other IO Register sets

	//TODO: Only use this when not covered by
	std::array<U8, IOREG_SIZE>
		backup{};
};