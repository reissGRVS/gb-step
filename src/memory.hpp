#include <array>
#include <memory>
#include <cstdint>

class Memory
{
	public:
		Memory(std::string biosPath, std::string romPath);
		
	private:

		// https://problemkaputt.de/gbatek.htm#gbamemorymap
		struct MemoryMap 
		{
			struct GeneralInternal
			{
				std::array<std::uint8_t, 0x04000> bios{};
				std::array<std::uint8_t, 0x40000> wramb{};
				std::array<std::uint8_t, 0x08000> wramc{};
				std::array<std::uint8_t, 0x003FF> ioreg{};
			} gen;

			struct InternalDisplay
			{
				std::array<std::uint8_t, 0x00400> pram{};
				std::array<std::uint8_t, 0x18000> vram{};
				std::array<std::uint8_t, 0x00400> oam{};
			} disp;

			struct External
			{
				//TODO: Make this as small as possible when rom is loaded?
				std::array<std::uint8_t, 0x2000000> rom{};
			} ext;
		} mem;
};
