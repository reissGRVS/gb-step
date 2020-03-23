#pragma once

#include <SFML/Audio.hpp>
#include "common/circular_queue.hpp"
#include "int.hpp"
#include "utils.hpp"

class AudioStream : public sf::SoundStream
{
public:
	static const U32 SAMPLE_FREQ = 32768;
	AudioStream()
	{
		initialize(2u, SAMPLE_FREQ);
	}
	bool playing = false;
	void PushOne(const S16& value)
	{
		samples.Push(value);	
	}

	static const U32 BUFFER_SIZE = SAMPLE_FREQ/2;
private:

    virtual bool onGetData(Chunk& data)
    {
		if (samples.Size() < (BUFFER_SIZE/2))
		{
			data.samples = emptyBuffer.data();
		 	data.sampleCount = BUFFER_SIZE/10;
		 	return true;
		}
		else
		{
			data.sampleCount = (BUFFER_SIZE/2);
			U16 i = 0;
			while (samples.IsEmpty() == false && i < (BUFFER_SIZE/2u))
			{
				buffer[i] = samples.Pop();
				i++;
			}
			data.samples = buffer.data();
			return true;
		}
    }

    virtual void onSeek(sf::Time)
    {
    }

	std::array<sf::Int16, BUFFER_SIZE> emptyBuffer = {0};
	std::array<sf::Int16, BUFFER_SIZE> buffer = {};
    CircularQueue<sf::Int16, BUFFER_SIZE> samples;
};