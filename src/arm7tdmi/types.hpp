#pragma once

#include <cstdint>
#include <utility>

using OpCode = std::uint32_t;
using BitSegment = std::pair<std::uint8_t, std::uint8_t>;
using ParamSegments = std::vector<BitSegment>;
using ParamList = std::vector<std::uint16_t>;
