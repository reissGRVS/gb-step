#pragma once

#include <array>
#include <memory>
#include <vector>

#include "types.hpp"
#include "../memory.hpp"
#include "registers.hpp"

namespace ARM7TDMI
{
	class CPU
	{
		public:
			CPU(std::shared_ptr<Memory> memory_) :
				memory(memory_)
			{};
			void Execute();
			RegisterSet registers;
		private:
			std::shared_ptr<Memory> memory;
			std::array<OpCode, 2> pipeline;
			
			#define ARM_OPCODES_INCLUDE_GUARD
			#include "arm_opcodes.hpp"
			#undef ARM_OPCODES_INCLUDE_GUARD
	};
}