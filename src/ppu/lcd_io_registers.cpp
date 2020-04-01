#include "platform/logging.hpp"
#include "ppu/ppu.hpp"

U32 PPU::Read(const AccessSize& size, U32 address, const Sequentiality&)
{
	U32 actualIndex = address - LCD_IO_START;
	auto value = ReadToSize(registers, actualIndex, size);

	return value;
}

void PPU::Write(const AccessSize& size, U32 address, U32 value, const Sequentiality&)
{
	U32 actualIndex = address - LCD_IO_START;

	if (size == Half) {
		switch (address) {
		case WIN0H:
			windows[WindowID::Win0].SetXValues(value);
			break;
		case WIN0V:
			windows[WindowID::Win0].SetYValues(value);
			break;
		case WIN1H:
			windows[WindowID::Win1].SetXValues(value);
			break;
		case WIN1V:
			windows[WindowID::Win1].SetYValues(value);
			break;
		case WININ: {
			windows[WindowID::Win0].SetSettings(BIT_RANGE(value, 0, 5));
			windows[WindowID::Win1].SetSettings(BIT_RANGE(value, 8, 13));
			break;
		}
		case WINOUT: {
			windows[WindowID::Outside].SetSettings(BIT_RANGE(value, 0, 5));
			windows[WindowID::Obj].SetSettings(BIT_RANGE(value, 8, 13));
			break;
		}
		case MOSAIC: {
			mosaic.bgHSize = BIT_RANGE(value, 0, 3) + 1;
			mosaic.bgVSize = BIT_RANGE(value, 4, 7) + 1;
			mosaic.objHSize = BIT_RANGE(value, 8, 11) + 1;
			mosaic.objVSize = BIT_RANGE(value, 12, 15) + 1;
		}
		default:
			break;
		}
	} else if (size == Byte && address >= WIN0H && address <= MOSAIC + 2) {
		LOG_ERROR("Byte window write needs implemented")
		exit(01);
	}
	WriteToSize(registers, actualIndex, value, size);
}