#pragma once

#include "memory/memory.hpp"
#include "arm7tdmi/irq_channel.hpp"
#include "ppu/bg_control_info.hpp"
#include "ppu/blend_control.hpp"
#include "ppu/lcd_io_registers.hpp"
#include "ppu/obj_attributes.hpp"
#include "ppu/tile_info.hpp"
#include "screen.hpp"
#include "utils.hpp"
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
		std::shared_ptr<IRQChannel> irqChannel_,
		std::function<void(bool)> HBlankCallback_,
		std::function<void(bool)> VBlankCallback_)
		: memory(memory_)
		, screen(screen_)
		, irqChannel(irqChannel_)
		, HBlankCallback(HBlankCallback_)
		, VBlankCallback(VBlankCallback_)
	{
	}

	void Execute(U32 ticks);

	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality&) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality&) override;

private:

	using OptPixel = std::optional<U16>;

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
	void InitTempSprite(ObjAttributes objAttrs);
	void DrawObject(ObjAttributes objAttrs);
	void SetObjPixel(OptPixel& pixel, U32 fbPos, bool objectTransparency, U16 prio);

	std::array<OptPixel, 64*64> tempSprite = {};

	// Text Mode
	void TextBGLine(const uint32_t& BG_ID);
	U32 GetScreenAreaOffset(U32 mapX,
		U32 mapY,
		U8 screenSize);

	// Rot Scale Mode
	void RotScaleBGLine(const U32& BG_ID);
	std::array<S32, 2> bgXRef = {}, bgYRef = {};
	U8 eva, evb, evy;
	BldCnt bldCnt;

	// Draw Utils

	U16 GetBgColorFromSubPalette(const U8& paletteNumber,
		const U8& colorID,
		bool obj = false);
	U16 GetBgColorFromPalette(const U8& colorID,
		bool obj = false);

	void SetSFXPixel(OptPixel& firstPrioPixel, OptPixel& secondPrioPixel, U16& dest, BldCnt::ColorSpecialEffect effect);
	
	template <typename T>
	void FetchDecode8BitPixel(U32 address, T& dest, bool obj)
	{
		auto pixelPalette = memory->GetByte(address);
		if (pixelPalette != 0)
		{
			dest = GetBgColorFromPalette(pixelPalette, obj);
		}
	}

	template <typename T>
	void FetchDecode4BitPixel(U32 address, T& dest, U8 paletteNumber, bool evenPixel, bool obj)
	{
		auto pixelPalette = memory->GetByte(address);
		if (evenPixel) {
			pixelPalette = BIT_RANGE(pixelPalette, 0, 3);
		} else {
			pixelPalette = BIT_RANGE(pixelPalette, 4, 7);
		}

		if (pixelPalette != 0)
		{
			dest = GetBgColorFromSubPalette(paletteNumber, pixelPalette, obj);
		}
	}

	std::shared_ptr<Memory> memory;
	Screen& screen;
	std::shared_ptr<IRQChannel> irqChannel;
	std::function<void(bool)> HBlankCallback;
	std::function<void(bool)> VBlankCallback;

	
	struct ObjPixel {
		OptPixel pixel;
		U16 prio;
		bool transparency;
	};

	const ObjPixel emptyObjPixel {{}, 5, false};

	std::array<ObjPixel, Screen::SCREEN_TOTAL> objFb;
	std::array<std::array<OptPixel, Screen::SCREEN_WIDTH>, 4> rows{};

	Screen::Framebuffer fb{};
	State state = Visible;
	U32 tickCount = 0;

	const U16 TILE_PIXEL_HEIGHT = 8, TILE_PIXEL_WIDTH = 8,
			  TILE_AREA_HEIGHT = 32, TILE_AREA_WIDTH = 32;

	const U32 TILE_AREA_ADDRESS_INC = 0x800, OBJ_START_ADDRESS = 0x06010000;

	const U32 BGCNT[4] = { BG0CNT, BG1CNT, BG2CNT, BG3CNT };

	const U16 MAX_DEPTH = 4;
};
