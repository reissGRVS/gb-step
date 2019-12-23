#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <vector>
namespace ARM7TDMI {

using OpCode = std::uint32_t;
using BitSegment = std::pair<std::uint8_t, std::uint8_t>;
using ParamSegments = std::vector<BitSegment>;

// Params should be stored from LSB to MSB
using ParamList = std::vector<std::uint32_t>;

using RegisterView = std::array<std::uint64_t, 16>;

} // namespace ARM7TDMI
