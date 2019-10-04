#include <iostream>
#include <string>

#include "gba.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Entering" << std::endl;
	if (argc != 3)
	{
		std::cerr << "Wrong number of args";
		//TODO: Log wrong number args
		return -1;
	}

	std::cout << "Populate config" << std::endl;
	GBAConfig cfg;
	cfg.biosPath = argv[1];
	cfg.romPath = argv[2];

	std::cout << "Create gba" << std::endl;

	GBA gba(cfg);
			/*
	
	gba.Run();
	*/
}