#include "TextureManager.h"
#define INVERTED_BIT            (1 << 5)
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
		__debugbreak();
	}


	fread(&pTGAfile->idLength, sizeof(unsigned char), 1, pFile);
	fread(&pTGAfile->colorMapType, sizeof(unsigned char), 1, pFile);

	//image type, only handle 2-11;
	fread(&pTGAfile->imageType, sizeof(unsigned char), 1, pFile);
	if (pTGAfile->imageType != 2 && pTGAfile->imageType != 10 && pTGAfile->imageType != 3)
	{
		fclose(pFile);
		std::printf("error, roor");
		__debugbreak();
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

	char buffer[255];
	fread(buffer, sizeof(unsigned char), pTGAfile->idLength, pFile);


	//For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	colorMode = pTGAfile->bitCount / 8;
	imageSize = pTGAfile->imageWidth * pTGAfile->imageHeight * colorMode;
	int size = imageSize;
	pTGAfile->imageDataBuffer.resize(imageSize);

	//reading another arbitrary byte

	//allocating memory for the data
	//pTGAfile->imageDataBuffer2 = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);
	m_pixelData.resize(imageSize);

	// Read the image data.
	fread(&pTGAfile->imageDataBuffer[0], sizeof(unsigned char), imageSize, pFile);
	
	for (int y = 0; y < pTGAfile->imageHeight; y++)
	{
		
		for (int x = 0; x < pTGAfile->imageWidth; x++)
		{
			int index = y * pTGAfile->imageWidth + x;

			if (!(pTGAfile->imageDescr & INVERTED_BIT))
			{
				index = ((pTGAfile->imageHeight) - 1 - y) * (pTGAfile->imageWidth) + x;
			}

			m_pixelData[index].B = pTGAfile->imageDataBuffer[index * colorMode];
			m_pixelData[index].G = pTGAfile->imageDataBuffer[index * colorMode + 1];
			m_pixelData[index].R = pTGAfile->imageDataBuffer[index * colorMode + 2];
			if (pTGAfile->bitCount == 32)
			{
				m_pixelData[index].A = pTGAfile->imageDataBuffer[index * colorMode + 3];
			}
		}
	}
	mp_tgaFile = pTGAfile;
	
	//delete pTGAfile;
	
	fclose(pFile);

}

