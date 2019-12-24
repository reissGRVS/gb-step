#include "memory/read_write_interface.hpp"

uint32_t ReadWriteInterface::ReadToSize(U8* byte, AccessSize size)
{
	auto word = reinterpret_cast<U32*>(byte);
	return (*word) & size;
}

void ReadWriteInterface::WriteToSize(U8* byte,
	U32 value,
	AccessSize size)
{
	switch (size) {
	case AccessSize::Byte: {
		(*byte) = value;
		break;
	}
	case AccessSize::Half: {
		auto half = reinterpret_cast<U16*>(byte);
		(*half) = value;
		break;
	}
	case AccessSize::Word: {
		auto word = reinterpret_cast<U32*>(byte);
		(*word) = value;
		break;
	}
	}
}