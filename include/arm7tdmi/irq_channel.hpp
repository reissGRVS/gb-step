#pragma once

enum Interrupt {
	VBlank = 0,
	HBlank = 1,
	VCounter = 2,
	Timer0 = 3,
	Timer1 = 4,
	Timer2 = 5,
	Timer3 = 6,
	Serial = 7,
	DMA0 = 8,
	DMA1 = 9,
	DMA2 = 10,
	DMA3 = 11,
	Keypad = 12,
	GamePak = 13
};

class IRQChannel {
	public:
		virtual void RequestInterrupt(Interrupt i) = 0;		
};