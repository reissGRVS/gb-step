#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "../memory.hpp"
#include "registers.hpp"

namespace ARM7TDMI
{
	using OpCode = std::uint32_t;
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
	};
}