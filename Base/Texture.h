#pragma once
#include "IResource.h"
#include <SFML/Graphics.hpp>

class Texture : public IResource
{
	sf::Texture m_Texture;

public:
	inline Texture(int width, int height, unsigned char* pixelData)
	{
		sf::Image image;
		image.create(width, height, pixelData);
		m_Texture.loadFromImage(image, sf::IntRect());
        m_Texture.setRepeated(true);
        m_Texture.setSmooth(false);
        m_Texture.setSrgb(false);
        m_Texture.generateMipmap();
	}

	const sf::Texture& GetSFTexture();

	virtual void Release();
};
