#pragma once

#include "int.hpp"
#include <array>
#include <string>

enum Sequentiality { NSEQ,
	SEQ,
	FREE };

enum AccessSize { Byte = 0xFFu,
	Half = 0xFFFFu,
	Word = 0xFFFFFFFFu };
template <std::size_t SIZE>
uint32_t ReadToSize(std::array<U8, SIZE>& arr,
	U32 address, const AccessSize& size)
{

	switch (size) {
	case AccessSize::Byte: {
		// if (address >= arr.size()) {
		// 	spdlog::get("std")->error("Out of bounds byte read @ {:X} in {}", address, Name());
		// 	exit(20);
		// }
		return arr[address];
	}
	case AccessSize::Half: {
		// if (address + 1 >= arr.size()) {
		// 	spdlog::get("std")->error("Out of bounds half read @ {:X} in {}", address, Name());
		// 	exit(20);
		// }
		return (arr[address] + (arr[address + 1] << 8));
	}
	case AccessSize::Word: {
		// if (address + 3 >= arr.size()) {
		// 	spdlog::get("std")->error("Out of bounds word read @ {:X} in {}", address, Name());
		// 	exit(20);
		// }
		return (arr[address] + (arr[address + 1] << 8)
			+ (arr[address + 2] << 16) + (arr[address + 3] << 24));
	}
	}
	//spdlog::get("std")->error("Read to size no match");
	exit(1);
}

template <std::size_t SIZE>
void WriteToSize(std::array<U8, SIZE>& arr,
	U32 address, U32 value, const AccessSize& size)
{
	switch (size) {
	case AccessSize::Byte: {
		// if (address >= arr.size()) {
		// 	spdlog::get("std")->error("Out of bounds byte write @ {:X} in {}", address, Name());
		// 	exit(20);
		// }
		arr[address] = (U8)value;
		break;
	}
	case AccessSize::Half: {
		// if (address + 1 >= arr.size()) {
		// 	spdlog::get("std")->error("Out of bounds half write @ {:X} in {}", address, Name());
		// 	exit(20);
		// }
		arr[address] = (U8)value;
		value >>= 8;
		arr[address + 1] = (U8)value;
		break;
	}
	case AccessSize::Word: {
		// if (address + 3 >= arr.size()) {
		// 	spdlog::get("std")->error("Out of bounds word write @ {:X} in {}", address, Name());
		// 	exit(20);
		// }

		arr[address] = (U8)value;
		value >>= 8;
		arr[address + 1] = (U8)value;
		value >>= 8;
		arr[address + 2] = (U8)value;
		value >>= 8;
		arr[address + 3] = (U8)value;
		break;
	}
	}
}

class ReadWriteInterface {

public:
	virtual std::string Name() = 0;
	virtual U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality& type)
		= 0;
	virtual void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality& type)
		= 0;


protected:
	virtual ~ReadWriteInterface() = default;
};