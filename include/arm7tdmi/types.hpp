#pragma once

#include "int.hpp"
#include <array>
#include <utility>
#include <vector>
namespace ARM7TDMI {

using OpCode = U32;
using BitSegment = std::pair<U8, U8>;
using ParamSegments = std::vector<BitSegment>;

// Params should be stored from LSB to MSB
using ParamList = std::array<U32, 10>;

using RegisterView = std::array<std::uint64_t, 16>;

} // namespace ARM7TDMI
