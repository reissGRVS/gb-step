#pragma once

template <class T, size_t N>
class CircularQueue {
	public:

		bool IsEmpty()
		{
			return elems_count == 0;
		}

		bool IsFull()
		{
			return elems_count == N;
		}

		T Pop()
		{
			if (IsEmpty())
				return 0;

			T val = elems[startIndex];
			startIndex++;
			if (startIndex == N)
			{
				startIndex = 0;
			}

			elems_count--;
			return val;
		}

		void Push(T val)
		{
			if (IsFull())
				return;

			elems[endIndex] = val;
			endIndex++;
			if (endIndex == N)
			{
				endIndex = 0;
			}
			elems_count++;
		}

	private:
		size_t elems_count = 0;
		size_t startIndex = 0;
		size_t endIndex = 0;
		T elems[N];
};
