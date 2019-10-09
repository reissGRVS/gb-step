#pragma once

#include <array>
#include <cstdint>
#include <memory>

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

			RegisterSet registers;
		private:
			std::shared_ptr<Memory> memory;
	};
}