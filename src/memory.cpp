#include "memory.hpp"

#include <iostream>
#include <fstream>

#include "spdlog/spdlog.h"

Memory::Memory(std::string biosPath, std::string romPath)
{
	//Read BIOS
	{
		std::ifstream infile(biosPath);
		if (!infile.is_open()){
			spdlog::error("BIOS not found at supplied path");
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.gen.bios.size())
		{
			//TODO: Log bios too big
			spdlog::error("BIOS too big");
			exit(-1);
		}
		infile.read(reinterpret_cast<char *>(mem.gen.bios.data()), length);
	}
	//Read ROM
	{
		std::ifstream infile(romPath);
		if (!infile.is_open()){
			spdlog::error("ROM not found at supplied path");
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.ext.rom.size())
		{
			spdlog::error("ROM too big");
			exit(-1);
		}
		infile.read(reinterpret_cast<char *>(mem.ext.rom.data()), length);
	}
}