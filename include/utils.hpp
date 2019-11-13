#pragma once

#define NBIT_MASK(N) ((N == 32) ? 0xFFFFFFFF : ((1 << N) - 1))

#define BIT_SET(V, N) (V |= (1 << N))
#define BIT_CLEAR(V, N) (V &= ~(1 << N))