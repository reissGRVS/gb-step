#pragma once

#include "int.hpp"
#include "memory/cart_backup.hpp"
#include "memory/regions.hpp"
#include <array>

class SRAM : public CartBackup {
public:

	SRAM(std::string saveFilePath)
	: saveFilePath(saveFilePath)
	{
		std::cout << saveFilePath << std::endl;
		std::ifstream infile(saveFilePath);
		if (!infile.is_open()) {
			return;
		}
		infile.seekg(0, infile.beg);
		infile.read(reinterpret_cast<char *>(sram.data()), SRAM_SIZE);
	}

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
	void Save() override {
		std::ofstream outfile (saveFilePath, std::ofstream::binary);
		outfile.write(reinterpret_cast<char *>(sram.data()), SRAM_SIZE);
		outfile.close();
	}

	virtual ~SRAM() = default;

private:

    std::string saveFilePath;
	std::array<U8, SRAM_SIZE> sram{};
};