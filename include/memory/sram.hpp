#pragma once

#include "int.hpp"
#include "memory/cart_backup.hpp"
#include "memory/regions.hpp"
#include <array>

class SRAM : public CartBackup {
public:
	U8 Read(U32 address) override
	{
		address &= address & SRAM_MASK;
		return sram[address];
	}
	void Write(U32 address, U8 value) override
	{
		address &= address & SRAM_MASK;
		sram[address] = value;
	}

	virtual ~SRAM() = default;

private:
	std::array<U8, SRAM_SIZE> sram{};
};