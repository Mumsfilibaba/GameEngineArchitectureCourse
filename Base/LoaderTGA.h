#pragma once

#include "ILoader.h"
#include "Texture.h"
#include <SFML/Graphics.hpp>


typedef struct
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

} TGAHeader;

class LoaderTGA: public ILoader
{
public:

	LoaderTGA();
	~LoaderTGA();
	IResource* LoadFromDisk(const std::string& file);
	IResource* LoadFromMemory(void* data, size_t size);
	size_t WriteToBuffer(const std::string& file, void* buffer);

private:
	void ReadFromDisk(const std::string& file, TGAHeader& pTGAfile);
};