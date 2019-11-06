#pragma once

#include <cstdint>
#include <utility>

namespace ARM7TDMI {

using OpCode = std::uint32_t;
using BitSegment = std::pair<std::uint8_t, std::uint8_t>;
using ParamSegments = std::vector<BitSegment>;

// Params should be stored from LSB to MSB
using ParamList = std::vector<std::uint32_t>;

}  // namespace ARM7TDMI
