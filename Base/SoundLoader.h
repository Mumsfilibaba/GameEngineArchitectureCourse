#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/InputStream.hpp>

class SoundStream : sf::InputStream
{
public:
	~SoundStream() override;
	sf::Int64 read(void* data, sf::Int64 size) override;
	sf::Int64 seek(sf::Int64 position) override;
	sf::Int64 tell() override;
	sf::Int64 getSize() override;
};

class SoundLoader
{
public:
	~SoundLoader();

	void LoadSound(void* pData, size_t dataSize);

private:
	SoundLoader();

public:
	static SoundLoader& GetInstance()
	{
		static SoundLoader soundLoader;
		return soundLoader;
	}
};