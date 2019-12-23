#pragma once

#include <array>
#include "int.hpp"
#include <iomanip>
#include <iostream>
#include <utility>

namespace ARM7TDMI {

class OpBacktrace {
	static const U32 BUFFER_SIZE = 10;
	using PCVal = U32;
	using OpcodeVal = U32;
	using OpcodePCPair = std::pair<PCVal, OpcodeVal>;

public:
	void addOpPCPair(PCVal PC, OpcodeVal Opcode)
	{
		auto pair = std::make_pair(PC, Opcode);
		last = (last + 1) % BUFFER_SIZE;
		buffer[last] = pair;
	}

	void printBacktrace()
	{
		for (auto i = 0; i != BUFFER_SIZE; i++) {
			int index = last - i;
			if (index < 0)
				index += BUFFER_SIZE;
			auto pair = buffer[index];
			std::cout << std::hex << "PC:0x" << pair.first << " Opcode:0x"
					  << pair.second << std::endl;
		}
	}

private:
	U32 last = 0;
	std::array<OpcodePCPair, BUFFER_SIZE> buffer{};
};

} // namespace ARM7TDMI
