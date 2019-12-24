#pragma once

#include "int.hpp"

enum Sequentiality { NSEQ,
	SEQ,
	FREE };

enum AccessSize { Byte = 0xFFu,
	Half = 0xFFFFu,
	Word = 0xFFFFFFFFu };

class ReadWriteInterface {

public:
	virtual U32 Read(AccessSize size,
		U32 address,
		Sequentiality type)
		= 0;
	virtual void Write(AccessSize size,
		U32 address,
		U32 value,
		Sequentiality type)
		= 0;

	uint32_t ReadToSize(U8* byte, AccessSize size);
	void WriteToSize(U8* byte,
		U32 value,
		AccessSize size);
};