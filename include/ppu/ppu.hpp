#pragma once

#include "memory/memory.hpp"
#include "ppu/bg_control_info.hpp"
#include "ppu/lcd_io_registers.hpp"
#include "ppu/obj_attributes.hpp"
#include "ppu/tile_info.hpp"
#include "screen.hpp"

#include "memory/regions.hpp"

#include <optional>
#include <vector>

#define GET_HALF(A) (U16)Read(Half, A, FREE)
class PPU : public LCDIORegisters {
	enum State { Visible,
		HBlank,
		VBlank };

public:
	PPU(std::shared_ptr<Memory> memory_, Screen& screen_,
		std::function<void(bool)> HBlankCallback_,
		std::function<void(bool)> VBlankCallback_)
		: memory(memory_)
		, screen(screen_)
		, HBlankCallback(HBlankCallback_)
		, VBlankCallback(VBlankCallback_)
	{
	}

	void Execute(U32 ticks);

	U32 Read(AccessSize size,
		U32 address,
		Sequentiality) override;
	void Write(AccessSize size,
		U32 address,
		U32 value,
		Sequentiality) override;

private:
	// State Management
	void ToHBlank();
	void OnHBlankFinish();
	void ToVBlank();
	void OnVBlankLineFinish();
	U16 GetDispStat(U8 bit);
	void UpdateDispStat(U8 bit, bool set);
	U16 IncrementVCount();

	// Draw Control
	void MergeRows(std::vector<uint8_t>& bgOrder);
	void DrawLine();
	uint8_t GetLayerPriority(uint8_t layer);
	std::vector<uint8_t> GetBGDrawOrder(std::vector<uint8_t> layers,
		uint8_t screenDisplay);

	// Objects
	void DrawObjects();
	void DrawObject(ObjAttributes objAttrs);
	void DrawTile(const TileInfo& info);

	// Text Mode
	void TextBGLine(const uint32_t& BG_ID);
	U32 GetScreenAreaOffset(U32 mapX,
		U32 mapY,
		U8 screenSize);

	using OptPixel = std::optional<U16>;
	// Draw Utils
	OptPixel GetTilePixel(U16 tileNumber,
		U16 x,
		U16 y,
		U16 colorDepth,
		U32 tileDataBase,
		bool verticalFlip,
		bool horizontalFlip,
		U16 paletteNumber,
		bool obj);
	U16 GetBgColorFromSubPalette(const U32& paletteNumber,
		const U32& colorID,
		bool obj = false);
	U16 GetBgColorFromPalette(const U32& colorID,
		bool obj = false);

	std::shared_ptr<Memory> memory;
	Screen& screen;
	std::function<void(bool)> HBlankCallback;
	std::function<void(bool)> VBlankCallback;

	Screen::Framebuffer depth{ 4 };
	std::array<std::array<OptPixel, Screen::SCREEN_WIDTH>, 4> rows{};
	Screen::Framebuffer fb{};
	State state = Visible;
	U32 tickCount = 0;

	const U16 TILE_PIXEL_HEIGHT = 8, TILE_PIXEL_WIDTH = 8,
			  TILE_AREA_HEIGHT = 32, TILE_AREA_WIDTH = 32;

	const U32 TILE_AREA_ADDRESS_INC = 0x800, BYTES_PER_ENTRY = 2,
			  OBJ_START_ADDRESS = 0x06010000;

	const U32 BGCNT[4] = { BG0CNT, BG1CNT, BG2CNT, BG3CNT };

	const U16 MAX_DEPTH = 4;
};
