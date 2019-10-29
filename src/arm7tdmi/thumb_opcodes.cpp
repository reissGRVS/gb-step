#include "cpu.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

// TODO: TIMINGS
namespace ARM7TDMI {

std::function<void()> CPU::ThumbOperation(OpCode opcode) {
  switch (opcode >> 13) {
	case 0b000: {
	  if (opcode && 0x1800) {
		// TODO: Op2
	  } else {
		// TODO: Op1
	  }
	  break;
	}
	case 0b001: {
	  // TODO: Op3
	  break;
	}
	case 0b010: {
	  switch (opcode >> 10 & BIT_MASK(3)) {
		case 0b000: {
		  // TODO Op4
		  break;
		}
		case 0b001: {
		  // TODO Op5
		  break;
		}
		case 0b010:
		case 0b011: {
		  // TODO Op6
		  break;
		}
		default: {
		  if (opcode >> 9 & BIT_MASK(1)) {
			// TODO Op8
		  } else {
			// TODO Op7
		  }
		}
	  }
	}
	case 0b011: {
	  // TODO: Op9
	  break;
	}
	case 0b100: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		// TODO: Op11
	  } else {
		// TODO: Op10
	  }
	  break;
	}
	case 0b101: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		if (opcode >> 8 & BIT_MASK(4)) {
		  // TODO: Op14
		} else {
		  // TODO: Op13
		}
	  } else {
		// TODO: Op12
	  }
	  break;
	}
	case 0b110: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		if ((opcode >> 8 & BIT_MASK(4)) == BIT_MASK(4)) {
		  // TODO: Op17
		} else {
		  // TODO: Op16
		}
	  } else {
		// TODO: Op15
	  }
	  break;
	}
	case 0b111: {
	  if (opcode >> 12 & BIT_MASK(1)) {
		// TODO: Op19
	  } else {
		// TODO: Op18
	  }
	  break;
	}

	default:
	  spdlog::error("Invalid Thumb Instruction: This should never happen");
	  break;
  }
}

}