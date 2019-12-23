#pragma once

#include "int.hpp"
#include <array>
#include <functional>
#include <memory>
#include <string>

#include "arm7tdmi/types.hpp"

#include "joypad.hpp"
#include "memory/cart_backup.hpp"
#include "memory/regions.hpp"
#include "memory/types.hpp"
#include "system_clock.hpp"
#include "utils.hpp"

class Memory {
public:
	Memory(std::shared_ptr<SystemClock> clock,
		std::string biosPath,
		std::string romPath,
		Joypad& joypad);

	U32 Read(AccessSize size,
		U32 address,
		Sequentiality type);
	void Write(AccessSize size,
		U32 address,
		U32 value,
		Sequentiality type);

	U8 GetByte(const U32& address)
	{
		return Read(AccessSize::Byte, address, Sequentiality::FREE);
	}
	U16 GetHalf(const U32& address)
	{
		return Read(AccessSize::Half, address, Sequentiality::FREE);
	}
	U32 GetWord(const U32& address)
	{
		return Read(AccessSize::Word, address, Sequentiality::FREE);
	}
	void SetByte(const U32& address, const U8& value)
	{
		Write(AccessSize::Byte, address, value, Sequentiality::FREE);
	}
	void SetHalf(const U32& address, const U16& value)
	{
		Write(AccessSize::Half, address, value, Sequentiality::FREE);
	}
	void SetWord(const U32& address, const U32& value)
	{
		Write(AccessSize::Word, address, value, Sequentiality::FREE);
	}

	void RequestInterrupt(Interrupt i)
	{
		auto intReq = GetHalf(IF);
		BIT_SET(intReq, i);
		SetHalf(IF, intReq);
	}

	void SetIOWriteCallback(U32 address,
		std::function<void(U32)> callback);
	void SetDebugWriteCallback(std::function<void(U32)> callback);

private:
	std::shared_ptr<SystemClock> clock;
	std::unordered_map<U32, std::function<void(U32)>>
		ioCallbacks;

	std::string FindBackupID(size_t length);

	U32 ReadToSize(U8* byte, AccessSize size);
	void WriteToSize(U8* byte, U32 value, AccessSize size);

	void Tick(AccessSize size, U32 page, Sequentiality seq);
	void TickBySize(AccessSize size,
		U32 ticks8,
		U32 ticks16,
		U32 ticks32);
	// https://problemkaputt.de/gbatek.htm#gbamemorymap
	std::function<void(U32)> PublishWriteCallback;
	Joypad& joypad;
	struct MemoryMap {
		struct {
			std::array<U8, BIOS_SIZE> bios{};
			std::array<U8, WRAMB_SIZE> wramb{};
			std::array<U8, WRAMC_SIZE> wramc{};
			std::array<U8, IOREG_SIZE> ioreg{};
		} gen;

		struct {
			std::array<U8, PRAM_SIZE> pram{};
			std::array<U8, VRAM_SIZE> vram{};
			std::array<U8, OAM_SIZE> oam{};
		} disp;

		struct {
			std::array<U8, ROM_SIZE> rom{};
			std::unique_ptr<CartBackup> backup;
		} ext;
	} mem;
};
