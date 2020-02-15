#pragma once

#include <array>

#include "arm7tdmi/irq_channel.hpp"
#include "timers/timer.hpp"
#include "timers/timers_io_registers.hpp"
#include "utils.hpp"

class Timers : public TimersIORegisters {
public:
	Timers(std::shared_ptr<Memory> memory, std::shared_ptr<IRQChannel> irqChannel);
	virtual ~Timers() = default;
	void Update(const U32 ticks);
	void SetReloadValue(U8 id, U16 value);
	void TimerCntHUpdate(U8 id, U16 value);

	U32 Read(const AccessSize& size,
		U32 address,
		const Sequentiality&) override;
	void Write(const AccessSize& size,
		U32 address,
		U32 value,
		const Sequentiality&) override;

private:
	std::shared_ptr<Memory> memory;
	std::shared_ptr<IRQChannel> irqChannel;
	
	U32 heldTicks;
	U32 minTicks;
	bool ticksToOverflowKnown = false;

	const std::array<Interrupt, 4> timerInterrupts{
		Interrupt::Timer0, Interrupt::Timer1, Interrupt::Timer2,
		Interrupt::Timer3
	};

	std::array<Timer, 4> timers{
		Timer(0, TM0CNT_L, memory), Timer(1, TM1CNT_L, memory),
		Timer(2, TM2CNT_L, memory), Timer(3, TM3CNT_L, memory)
	};
};