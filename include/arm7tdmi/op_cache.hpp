#pragma once

#include "arm7tdmi/types.hpp"
#include "int.hpp"
#include <array>
#include <iostream>
#include <optional>
#include <unordered_map>

namespace ARM7TDMI {

struct OpInfo {
	Op op = nullptr;
	OpCode opcode = 0;
};

class OpCache {
public:
	void AddOp(Op newOp, const OpCode& opcode)
	{
		store.try_emplace(opcode, std::move(newOp));
	}

	bool LookupOp(const OpCode& opcode)
	{
		auto oplookup = store.find(opcode);
		if (oplookup != store.end()) {
			oplookup->second();
			return true;
		}

		return false;
	}

private:
	static const U16 STORE_SIZE = 10;
	std::unordered_map<OpCode, Op> store {};
};

}