#pragma once

#include <SFML/Audio.hpp>
#include "common/circular_queue.hpp"
#include "int.hpp"
#include "utils.hpp"
#include <iostream>

class AudioStream : public sf::SoundStream
{
public:
	static const U32 sampleFreq = 32768;
	AudioStream()
	{
		initialize(1u, sampleFreq);
	}
	bool playing = false;
	void PushOne(const S16& value)
	{
		if (!playing)
		{
			samples.Push(value * 10);
			if (samples.IsFull()) 
			{
				play();
				playing = true;
			}
		}
		

	}

	static const U32 BUFFER_SIZE = sampleFreq*10;
private:

    virtual bool onGetData(Chunk& data)
    {
		
		if (samples.IsFull())
		{
			data.sampleCount = BUFFER_SIZE;
			data.samples = samples.elems;
			samples.Reset();
			return true;
		}
		else
		{
			return false;
		}
		


		// if (samples.Size() < (BUFFER_SIZE/2))
		// {
		// 	data.samples = emptyBuffer.data();
		//  	data.sampleCount = BUFFER_SIZE/10;
		// 	std::cout << "empty" << std::endl;
		//  	return false;
		// }
		// else
		// {
		// 	data.sampleCount = (BUFFER_SIZE/2);
		// 	U16 i = 0;
		// 	while (samples.IsEmpty() == false && i < (BUFFER_SIZE/2u))
		// 	{
		// 		buffer[i] = samples.Pop();
		// 		i++;
		// 	}
		// 	data.samples = buffer.data();
		// 	return true;
		// }
    }

    virtual void onSeek(sf::Time)
    {
    }

	std::array<sf::Int16, BUFFER_SIZE> emptyBuffer = {0};
	std::array<sf::Int16, BUFFER_SIZE> buffer = {};
    CircularQueue<sf::Int16, BUFFER_SIZE> samples;
};