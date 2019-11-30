#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <fstream>
#include <iostream>
#include <vector>

struct TGAFile
{
	unsigned char idLength;
	unsigned char colorMapType;
	unsigned char imageType;
	short int imageWidth;
	short int imageHeight;
	unsigned char bitCount; //pixelDepth

	//image color and map data
	unsigned short startX;
	unsigned short startY;
	unsigned short firstPixel; //index of first entry;
	unsigned short nrOfPixels; //number of entries of the color map
	unsigned char bitsPerPixel; // number of bits/pixel
	unsigned char imageDescr;

	unsigned char nrOfBytes; // number of bytes the imageId field consists of

	std::vector<unsigned char> imageDataBuffer; // colorInformation about every pixel
	unsigned char* imageDataBuffer2;

};

struct PixelInfo
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;

};

class TextureManager
{

public:

	TGAFile* mp_tgaFile;
	std::vector<PixelInfo> m_pixelData;

private:


public:
	//maybe make it return the TGAFile-pointer
	void LoadTGAFile(const char* fileName);

	TextureManager() = default;
	~TextureManager() = default;


};


#endif // !TEXTUREMANAGER_H
