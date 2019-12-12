
#pragma once

#include <cstdint>

class SystemClock {
 public:
  void Tick(std::uint32_t ticks) { total += ticks; }

  std::uint32_t SinceLastCheck() {
	auto since = total - lastCheck;
	lastCheck = total;
	return since;
  }

 private:
  std::uint32_t total = 0;
  std::uint32_t lastCheck = 0;
};