#include "TextureManager.h"

void TextureManager::LoadTGAFile(char* fileName, TGAFile file)
{

	FILE* pFile = nullptr;
	unsigned char byteCharEater;
	unsigned short byteShortEater;
	long imageSize;
	int colorMode;

	// reading file with binary mode.
	pFile = fopen(fileName, "rb");
	if (!pFile)
	{
		//make an assert
		std::printf("could not open TGA-File");
	}


	fread(&byteCharEater, sizeof(unsigned char), 1, pFile);
	fread(&byteCharEater, sizeof(unsigned char), 1, pFile);

	//image typ, only handle 2-11;
	fread(&file.imageType, sizeof(unsigned char), 1, pFile);
	if (file.imageType != 2 && file.imageType != 3)
	{
		fclose(pFile);
		std::printf("error, roor");
	}

	// Read 13 bytes of data we don't need.
	fread(&byteShortEater, sizeof(short int), 1, pFile);
	fread(&byteShortEater, sizeof(short int), 1, pFile);
	fread(&byteCharEater, sizeof(unsigned char), 1, pFile);
	fread(&byteShortEater, sizeof(short int), 1, pFile);
	fread(&byteShortEater, sizeof(short int), 1, pFile);

	// Read the image's width and height.
	fread(&file.imageWidth, sizeof(short int), 1, pFile);
	fread(&file.imageHeight, sizeof(short int), 1, pFile);

	fread(&file.pixelDepth, sizeof(unsigned char), 1, pFile);
	//  For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	colorMode = 3;
	imageSize = file.imageWidth * file.imageHeight * colorMode;

	//reading another arbitrary byte
	fread(&byteCharEater, sizeof(unsigned char), 1, pFile);


	// Allocate memory for the image data.
	file.imageDataBuffer = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);
	// Read the image data.
	fread(file.imageDataBuffer, sizeof(unsigned char), imageSize, pFile);

	fclose(pFile);
}

