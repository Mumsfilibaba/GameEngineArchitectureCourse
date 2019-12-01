#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>

struct TGAFile
{
	unsigned char idLength;
	unsigned char colorMapType;
	unsigned char imageType;
	unsigned short imageWidth;
	unsigned short imageHeight;
	unsigned char bitCount; //pixelDepth

	//image color and map data
	unsigned short startX;
	unsigned short startY;
	unsigned short firstPixel; //index of first entry;
	unsigned short nrOfPixels; //number of entries of the color map
	unsigned char bitsPerPixel; // number of bits/pixel
	unsigned char imageDescr;

	unsigned char nrOfBytes; // number of bytes the imageId field consists of

	unsigned char* imageDataBuffer;

};


class TextureManager
{

public:

private:


public:
	//maybe make it return the TGAFile-pointer
	sf::Texture* LoadTGAFile(const char* fileName);
	sf::Texture* LoadTGAFile(void* pData);


	TextureManager() = default;
	~TextureManager() = default;


};


#endif // !TEXTUREMANAGER_H
