#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>

int total = 0;

enum ABC {
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P
};

template <uint8_t First, uint8_t Last>
struct static_for {
	template <typename Fn>
	void operator()(Fn const& fn) const
	{
		if (First < Last) {
			fn(First);
			static_for<First + 1, Last>()(fn);
		}
	}
};

template <uint8_t N>
struct static_for<N, N> {
	template <typename Fn>
	void operator()(Fn const& fn) const
	{
	}
};

typedef void (*switchN)();

template <uint8_t num>
static constexpr void switch0()
{
	switch (static_cast<ABC>(num)) {
	case A:
		total += num;
		break;
	case B:
		total += num;
		break;
	case C:
		total += num;
		break;
	case D:
		total += num;
		break;
	case E:
		total += num;
		break;
	case F:
		total += num;
		break;
	case G:
		total += num;
		break;
	case H:
		total += num;
		break;
	case I:
		total += num;
		break;
	case J:
		total += num;
		break;
	case K:
		total += num;
		break;
	case L:
		total += num;
		break;
	case M:
		total += num;
		break;
	case N:
		total += num;
		break;
	case O:
		total += num;
		break;
	case P:
		total += num;
		break;
	}
}

void switch16(uint8_t num)
{
	switch (static_cast<ABC>(num)) {
	case A:
		total += num;
		break;
	case B:
		total += num;
		break;
	case C:
		total += num;
		break;
	case D:
		total += num;
		break;
	case E:
		total += num;
		break;
	case F:
		total += num;
		break;
	case G:
		total += num;
		break;
	case H:
		total += num;
		break;
	case I:
		total += num;
		break;
	case J:
		total += num;
		break;
	case K:
		total += num;
		break;
	case L:
		total += num;
		break;
	case M:
		total += num;
		break;
	case N:
		total += num;
		break;
	case O:
		total += num;
		break;
	case P:
		total += num;
		break;
	}
}

int main()
{

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<uint8_t> dist16(0, 15);
	int loop;

	std::array<switchN, 16> switchNarr = { switch0<0>, switch0<1>, switch0<2>, switch0<3>,
		switch0<4>, switch0<5>, switch0<6>, switch0<7>,
		switch0<8>, switch0<9>, switch0<10>, switch0<11>,
		switch0<12>, switch0<13>, switch0<14>, switch0<15> };

	for (int rep = 0; rep < 10; rep++) {
		loop = 1000000;
		auto start1 = std::chrono::high_resolution_clock::now();
		while (loop--) {
			auto num = dist16(rng);
			switch16(num);
		}
		auto finish1 = std::chrono::high_resolution_clock::now();

		loop = 1000000;
		auto start2 = std::chrono::high_resolution_clock::now();
		while (loop--) {
			auto num = dist16(rng);
			switchNarr[num]();
		}
		auto finish2 = std::chrono::high_resolution_clock::now();
	case J:
		total += num;
		break;
	case K:
		total += num;
		break;
	case L:
		total += num;
		break;
	case M:
		total += num;
		break;
	case N:
		total += num;
		break;
	case O:
		total += num;
		break;
	case P:
		total += num;
		break;
	}
}

int main()
{

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<uint8_t> dist16(0, 15);
	int loop;

	std::array<switchN, 16> switchNarr = { switch0<0>, switch0<1>, switch0<2>, switch0<3>,
		switch0<4>, switch0<5>, switch0<6>, switch0<7>,
		switch0<8>, switch0<9>, switch0<10>, switch0<11>,
		switch0<12>, switch0<13>, switch0<14>, switch0<15> };

	for (int rep = 0; rep < 10; rep++) {
		loop = 1000000;
		auto start1 = std::chrono::high_resolution_clock::now();
		while (loop--) {
			auto num = dist16(rng);
			switch16(num);
		}
		auto finish1 = std::chrono::high_resolution_clock::now();

		loop = 1000000;
		auto start2 = std::chrono::high_resolution_clock::now();
		while (loop--) {
			auto num = dist16(rng);
			switchNarr[num]();
		}
		auto finish2 = std::chrono::high_resolution_clock::now();

		std::cout << std::endl;
		std::chrono::duration<double> elapsed1 = finish1 - start1;
		std::cout << "Elapsed time: " << elapsed1.count() << " s\n";
		std::chrono::duration<double> elapsed2 = finish2 - start2;
		std::cout << "Elapsed time: " << elapsed2.count() << " s\n";
	}
}
