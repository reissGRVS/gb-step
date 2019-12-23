#pragma once

#include "memory/cart_backup.hpp"
#include "memory/regions.hpp"
#include <array>
#include <cstdint>

class SRAM : public CartBackup {
public:
	std::uint8_t Read(std::uint32_t address) override
	{
		address &= address & SRAM_MASK;
		return sram[address];
	}
	void Write(std::uint32_t address, std::uint8_t value) override
	{
		address &= address & SRAM_MASK;
		sram[address] = value;
	}

private:
	std::array<std::uint8_t, SRAM_SIZE> sram{};
};