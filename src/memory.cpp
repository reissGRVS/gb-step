#include "memory.hpp"
#include <iostream>
#include <fstream>

Memory::Memory(std::string biosPath, std::string romPath)
{
	//Read BIOS
	{
		std::ifstream infile(biosPath);
		if (!infile.is_open()){
			std::cerr << "bios not found";
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.gen.bios.size())
		{
			//TODO: Log bios too big
			std::cerr << "bios too big";
			exit(-1);
		}
		infile.read(reinterpret_cast<char *>(mem.gen.bios.data()), length);
	}
	//Read ROM
	{
		std::ifstream infile(romPath);
		if (!infile.is_open()){
			std::cerr << "rom not found";
			exit(-1);
		}
		infile.seekg(0, infile.end);
		size_t length = infile.tellg();
		infile.seekg(0, infile.beg);
		if (length > mem.ext.rom.size())
		{
			//TODO: Log rom too big
			std::cerr << "rom too big";
			exit(-1);
		}
		infile.read(reinterpret_cast<char *>(mem.ext.rom.data()), length);
	}
}