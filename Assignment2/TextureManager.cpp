#include "TextureManager.h"
#include "Helpers.h"

void TextureManager::LoadTGAFile(const char* fileName)
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

	// Read 13 bytes of data we don't need.
	fread(&pTGAfile->firstPixel, sizeof(short int), 1, pFile);
	fread(&pTGAfile->nrOfPixels, sizeof(short int), 1, pFile);
	fread(&pTGAfile->bitsPerPixel, sizeof(unsigned char), 1, pFile);
	fread(&pTGAfile->startX, sizeof(short int), 1, pFile);
	fread(&pTGAfile->startY, sizeof(short int), 1, pFile);

	// Read the image's width and height.
	fread(&pTGAfile->imageWidth, sizeof(short int), 1, pFile);
	fread(&pTGAfile->imageHeight, sizeof(short int), 1, pFile);

	fread(&pTGAfile->bitCount, sizeof(unsigned char), 1, pFile);

	//For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	colorMode = pTGAfile->bitCount/ 8;
	imageSize = pTGAfile->imageWidth * pTGAfile->imageHeight * colorMode;
	int size = imageSize;
	pTGAfile->imageDataBuffer.resize(imageSize);

	//reading another arbitrary byte
	fread(&pTGAfile->imageDescr, sizeof(unsigned char), 1, pFile);

	//allocating memory for the data
	//pTGAfile->imageDataBuffer2 = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

	// Read the image data.
	fread(&pTGAfile->imageDataBuffer[0], sizeof(unsigned char), imageSize, pFile);
	mp_tgaFile = pTGAfile;
	
	//delete pTGAfile;
	
	fclose(pFile);

}

