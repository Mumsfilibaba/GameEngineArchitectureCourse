#include "TextureManager.h"
#define INVERTED_BIT            (1 << 5)
#define BYTE32 32
#include "Helpers.h"
#include <iostream>

sf::Texture* TextureManager::LoadTGAFile(const char* fileName)
{
	TGAFile* pTGAfile = new TGAFile();
	FILE* pFile = nullptr;
	unsigned char byteCharEater;
	unsigned short byteShortEater;
	long imageSize;
	int colorMode;
	unsigned char colorSwap;


	// reading file with binary mode.
	pFile = fopen(fileName, "rb");
	if (!pFile)
	{
		//make an assert
		fclose(pFile);
		std::printf("could not open TGA-File");
		debugbreak();
	}


	fread(&pTGAfile->idLength, sizeof(unsigned char), 1, pFile);
	fread(&pTGAfile->colorMapType, sizeof(unsigned char), 1, pFile);

	//image type, only handle 2-11;
	fread(&pTGAfile->imageType, sizeof(unsigned char), 1, pFile);
	if (pTGAfile->imageType != 2 && pTGAfile->imageType != 10 && pTGAfile->imageType != 3)
	{
		fclose(pFile);
		std::printf("error, roor");
		debugbreak();
	}

	//field4 (5 bytes)
	fread(&pTGAfile->firstPixel, sizeof(short int), 1, pFile);
	fread(&pTGAfile->nrOfPixels, sizeof(short int), 1, pFile);
	fread(&pTGAfile->bitsPerPixel, sizeof(unsigned char), 1, pFile);

	//field5 10 bytes
	fread(&pTGAfile->startX, sizeof(short int), 1, pFile);
	fread(&pTGAfile->startY, sizeof(short int), 1, pFile);

	// Read the image's width and height.
	fread(&pTGAfile->imageWidth, sizeof(short int), 1, pFile);
	fread(&pTGAfile->imageHeight, sizeof(short int), 1, pFile);
	fread(&pTGAfile->bitCount, sizeof(unsigned char), 1, pFile);
	fread(&pTGAfile->imageDescr, sizeof(unsigned char), 1, pFile);

	//field 6 (byte length depending on IDLengths, max 255)
	char buffer[255];
	fread(buffer, sizeof(unsigned char), pTGAfile->idLength, pFile);


	//For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	colorMode = pTGAfile->bitCount / 8;
	imageSize = pTGAfile->imageWidth * pTGAfile->imageHeight * colorMode;
	int size = imageSize;
	

	//reading another arbitrary byte

	//allocating memory for the data
	pTGAfile->imageDataBuffer = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

	// Read the image data.
	//fread(&pTGAfile->imageDataBuffer[0], sizeof(unsigned char), imageSize, pFile);
	fread(pTGAfile->imageDataBuffer, sizeof(unsigned char), imageSize, pFile);
	
	
	for (int y = 0; y < pTGAfile->imageHeight; y++)
	{
		
		for (int x = 0; x < pTGAfile->imageWidth; x++)
		{
			int index = y * pTGAfile->imageWidth + x;

			//color swap 
			if (!(pTGAfile->imageDescr & INVERTED_BIT))
			{
				index = ((pTGAfile->imageHeight) - 1 - y) * (pTGAfile->imageWidth) + x;
			}

			unsigned char tempBlue;
			//swap from BGR to RGB
			tempBlue = pTGAfile->imageDataBuffer[index * colorMode];
			pTGAfile->imageDataBuffer[index * colorMode] = pTGAfile->imageDataBuffer[index * colorMode + 2];
			pTGAfile->imageDataBuffer[index * colorMode + 2] = tempBlue;
		}
	}
	
	sf::Image image;
	//make member variable later?
	sf::Texture* texture = new sf::Texture();
	
	image.create(pTGAfile->imageWidth, pTGAfile->imageHeight, pTGAfile->imageDataBuffer);
	texture->loadFromImage(image, sf::IntRect());
	//delete pTGAfile;
	fclose(pFile);
	
	return texture;

}

sf::Texture* TextureManager::LoadTGAFile(void* pData)
{
	TGAFile* pTGAfile = new TGAFile();


	long imageSize;
	int colorMode;
	unsigned char colorSwap;
	int imageDataIndex = 0;
	//need to know from what index the color map data is stored.

	size_t currentAddress = (size_t)pData;

	pTGAfile->idLength = *(unsigned char*)(currentAddress);
	pTGAfile->colorMapType = *(unsigned char*)(currentAddress += sizeof(pTGAfile->idLength));
	pTGAfile->imageType = *(unsigned char*)(currentAddress += sizeof(pTGAfile->imageType));

	if (pTGAfile->imageType != 2 && pTGAfile->imageType != 10 && pTGAfile->imageType != 3)
	{
		std::printf("error, roor");
		debugbreak();
	}

	//field4 (5 bytes)
	pTGAfile->firstPixel   = *(short int*)(currentAddress += sizeof(pTGAfile->imageType));
	pTGAfile->nrOfPixels   = *(short int*)(currentAddress += sizeof(pTGAfile->firstPixel));
	pTGAfile->bitsPerPixel = *(unsigned char*)(currentAddress += sizeof(pTGAfile->nrOfPixels));

	//field5 10 bytes
	pTGAfile->startX = *(short*)(currentAddress += sizeof(pTGAfile->bitsPerPixel));
	pTGAfile->startY =  *(short*)(currentAddress += sizeof(pTGAfile->startX));


	// Read the image's width and height.
	pTGAfile->imageWidth  = *(unsigned short*)(currentAddress += sizeof(pTGAfile->startY));
	pTGAfile->imageHeight = *(unsigned int*)(currentAddress += sizeof(pTGAfile->imageWidth));
	std::cout << "width: " << pTGAfile->imageWidth << std::endl;
	std::cout << "height: " << pTGAfile->imageHeight << std::endl;
	pTGAfile->bitCount	  = *(unsigned char*)(currentAddress += sizeof(pTGAfile->imageHeight));
	pTGAfile->imageDescr  = *(unsigned char*)(currentAddress += sizeof(pTGAfile->bitCount));
	currentAddress += sizeof(pTGAfile->imageDescr);
	//field 6 (byte length depending on IDLengths, max 255)


	currentAddress += pTGAfile->idLength;


	//For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	colorMode = pTGAfile->bitCount / 8;
	imageSize = pTGAfile->imageWidth * pTGAfile->imageHeight * colorMode;
	int size = imageSize;
	//pTGAfile->imageDataBuffer.resize(imageSize);

	//better to copy the data?!
	//allocating memory for the data
	pTGAfile->imageDataBuffer = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);
	memcpy(pTGAfile->imageDataBuffer, (void*)currentAddress, (size_t)imageSize);

	for (short int y = 0; y < pTGAfile->imageHeight; y++)
	{
		if (y == 845)
		{
			int as = 0;
		}
		for (short int x = 0; x < pTGAfile->imageWidth; x++)
		{
			int index = y * pTGAfile->imageWidth + x;
			//color swap 
			if (!(pTGAfile->imageDescr & INVERTED_BIT))
			{
				index = ((pTGAfile->imageHeight) - 1 - y) * (pTGAfile->imageWidth) + x;

			}
			if (x == 719)
			{
				int as = 0;
			}
			unsigned char tempBlue;

			tempBlue = pTGAfile->imageDataBuffer[index * colorMode];
			pTGAfile->imageDataBuffer[index * colorMode] = pTGAfile->imageDataBuffer[index * colorMode + 2];
			pTGAfile->imageDataBuffer[index * colorMode + 2] = tempBlue;
		}
	}

	sf::Image image;
	sf::Texture* texture = new sf::Texture();

	image.create(pTGAfile->imageWidth, pTGAfile->imageHeight, pTGAfile->imageDataBuffer);
	texture->loadFromImage(image, sf::IntRect());
	delete pTGAfile;
	
	return texture;

}

