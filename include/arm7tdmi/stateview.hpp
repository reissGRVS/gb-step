
#pragma once

#include "opbacktrace.hpp"
#include "types.hpp"

namespace ARM7TDMI {

struct StateView {
  RegisterView registers;
  OpBacktrace backtrace;
};

}  // namespace ARM7TDMI