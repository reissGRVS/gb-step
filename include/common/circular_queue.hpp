#pragma once

template <class T, size_t N>
class CircularQueue {
	public:


		void Reset()
		{
			elems_count = 0;
			startIndex = 0;
			endIndex = 0;
		}

		bool IsEmpty()
		{
			return elems_count == 0;
		}

		bool IsFull()
		{
			return elems_count == N;
		}

		size_t Size()
		{
			return elems_count;
		}

		T Pop()
		{
			if (IsEmpty())
				return (T) 0;

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

		T elems[N] = {0};
	private:
		size_t elems_count = 0;
		size_t startIndex = 0;
		size_t endIndex = 0;
};
