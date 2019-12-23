
#pragma once

#include "arm7tdmi/types.hpp"
#include "opbacktrace.hpp"

namespace ARM7TDMI {

struct StateView {
	RegisterView registers;
	OpBacktrace backtrace;
};

} // namespace ARM7TDMI