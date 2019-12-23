#include "memory/regions.hpp"
#include "ppu/ppu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

const std::uint16_t ROTSCALE_BGMAP_SIZES[4][2] = { { 128, 128 },
	{ 256, 256 },
	{ 512, 512 },
	{ 1024, 1024 } };