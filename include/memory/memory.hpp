#pragma once

#include "int.hpp"
#include <array>
#include <functional>
#include <memory>
#include <string>

#include "arm7tdmi/types.hpp"

#include "arm7tdmi/ir_io_registers.hpp"
#include "joypad.hpp"
#include "memory/cart_backup.hpp"
#include "memory/io_registers.hpp"
#include "memory/read_write_interface.hpp"
#include "memory/regions.hpp"
#include "system_clock.hpp"
#include "utils.hpp"

class Memory : public ReadWriteInterface {
public:
	Memory(std::shared_ptr<SystemClock> clock,
		std::string biosPath,
		std::string romPath,
		Joypad& joypad);

	std::string Name() { return "MEMORY"; };
	void Save() { mem.ext.backup->Save(); }
	void AttachIORegisters(std::shared_ptr<IORegisters> io);
	void AttachIRIORegisters(std::shared_ptr<IRIORegisters> irio);

	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality& type) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality& type) override;

	U8 GetByte(const U32& address);
	U16 GetHalf(const U32& address);
	U32 GetWord(const U32& address);
	void SetByte(const U32& address, const U8& value);
	void SetHalf(const U32& address, const U16& value);
	void SetWord(const U32& address, const U32& value);

	void SetIOWriteCallback(U32 address,
		std::function<void(U32)> callback);
	void SetDebugWriteCallback(std::function<void(U32)> callback);

private:
	std::shared_ptr<SystemClock> clock;
	std::unordered_map<U32, std::function<void(U32)>>
		ioCallbacks;

	std::string FindBackupID(size_t length);

	void Tick(const AccessSize& size, const U32& page, const Sequentiality& seq);
	void TickBySize(const AccessSize& size,
		const U32& ticks8,
		const U32& ticks16,
		const U32& ticks32);
	// https://problemkaputt.de/gbatek.htm#gbamemorymap
	std::function<void(U32)> PublishWriteCallback;
	Joypad& joypad;
	std::shared_ptr<IRIORegisters> irio;

	struct MemoryMap {
		struct {
			std::array<U8, BIOS_SIZE> bios {};
			std::array<U8, WRAMB_SIZE> wramb {};
			std::array<U8, WRAMC_SIZE> wramc {};
			std::shared_ptr<IORegisters> io {};
		} gen;

		struct {
			std::array<U8, PRAM_SIZE> pram {};
			std::array<U8, VRAM_SIZE> vram {};
			std::array<U8, OAM_SIZE> oam {};
		} disp;

		struct {
			std::array<U8, ROM_SIZE> rom {};
			std::unique_ptr<CartBackup> backup;
		} ext;
	} mem;
};
