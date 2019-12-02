#include "LoaderTGA.h"
#include "Helpers.h"

#define INVERTED_BIT            (1 << 5)
#define BYTE32 32

IResource* LoaderTGA::LoadFromDisk(const std::string& file)
{
	TGAHeader pTGAfile;
	ReadFromDisk(file, pTGAfile);
	Texture* pTexture = new Texture(pTGAfile.imageWidth, pTGAfile.imageHeight, pTGAfile.imageDataBuffer);
	free(pTGAfile.imageDataBuffer);
	return pTexture;
}

IResource* LoaderTGA::LoadFromMemory(void* data, size_t size)
{
	short int width = *(short int*)(data);
	short int height = *(short int*)((size_t)data + 2);

	return new Texture(width, height, (unsigned char*)((size_t)data + 4));
}

size_t LoaderTGA::WriteToBuffer(const std::string& file, void* buffer)
{
	TGAHeader pTGAfile;

	ReadFromDisk(file, pTGAfile);

	memcpy(buffer, &pTGAfile.imageWidth, sizeof(short int));
	memcpy((void*)((size_t)buffer + 2), &pTGAfile.imageHeight, sizeof(short int));
	memcpy((void*)((size_t)buffer + 4), pTGAfile.imageDataBuffer, (pTGAfile.imageWidth *  pTGAfile.imageHeight * 4));

	size_t sizeInBytes = ((pTGAfile.imageWidth *  pTGAfile.imageHeight * 4) + 2 + 2);
	free(pTGAfile.imageDataBuffer);

	return sizeInBytes;
}

void LoaderTGA::ReadFromDisk(const std::string& file, TGAHeader& pTGAfile)
{
	FILE* pFile = nullptr;
	unsigned char byteCharEater;
	unsigned short byteShortEater;
	long imageSize;
	int colorMode;
	unsigned char colorSwap;


	// reading file with binary mode.
	pFile = fopen(file.c_str(), "rb");
	if (!pFile)
	{
		//make an assert
		fclose(pFile);
		std::printf("could not open TGA-File");
		debugbreak();
	}


	fread(&pTGAfile.idLength, sizeof(unsigned char), 1, pFile);
	fread(&pTGAfile.colorMapType, sizeof(unsigned char), 1, pFile);

	//image type, only handle 2-11;
	fread(&pTGAfile.imageType, sizeof(unsigned char), 1, pFile);
	if (pTGAfile.imageType != 2 && pTGAfile.imageType != 10 && pTGAfile.imageType != 3)
	{
		fclose(pFile);
		std::printf("error, roor");
		debugbreak();
	}

	//field4 (5 bytes)
	fread(&pTGAfile.firstPixel, sizeof(short int), 1, pFile);
	fread(&pTGAfile.nrOfPixels, sizeof(short int), 1, pFile);
	fread(&pTGAfile.bitsPerPixel, sizeof(unsigned char), 1, pFile);

	//field5 10 bytes
	fread(&pTGAfile.startX, sizeof(short int), 1, pFile);
	fread(&pTGAfile.startY, sizeof(short int), 1, pFile);

	// Read the image's width and height.
	fread(&pTGAfile.imageWidth, sizeof(short int), 1, pFile);
	fread(&pTGAfile.imageHeight, sizeof(short int), 1, pFile);
	fread(&pTGAfile.bitCount, sizeof(unsigned char), 1, pFile);
	fread(&pTGAfile.imageDescr, sizeof(unsigned char), 1, pFile);

	//field 6 (byte length depending on IDLengths, max 255)
	char buffer[255];
	fread(buffer, sizeof(unsigned char), pTGAfile.idLength, pFile);


	//For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	colorMode = pTGAfile.bitCount / 8;
	imageSize = pTGAfile.imageWidth * pTGAfile.imageHeight * colorMode;
	int size = imageSize;


	//reading another arbitrary byte

	//allocating memory for the data
	pTGAfile.imageDataBuffer = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

	// Read the image data.
	//fread(&pTGAfile->imageDataBuffer[0], sizeof(unsigned char), imageSize, pFile);
	fread(pTGAfile.imageDataBuffer, sizeof(unsigned char), imageSize, pFile);


	for (int y = 0; y < pTGAfile.imageHeight; y++)
	{

		for (int x = 0; x < pTGAfile.imageWidth; x++)
		{
			int index = y * pTGAfile.imageWidth + x;

			//color swap 
			if (!(pTGAfile.imageDescr & INVERTED_BIT))
			{
				index = ((pTGAfile.imageHeight) - 1 - y) * (pTGAfile.imageWidth) + x;
			}

			unsigned char tempBlue;
			//swap from BGR to RGB
			tempBlue = pTGAfile.imageDataBuffer[index * colorMode];
			pTGAfile.imageDataBuffer[index * colorMode] = pTGAfile.imageDataBuffer[index * colorMode + 2];
			pTGAfile.imageDataBuffer[index * colorMode + 2] = tempBlue;
		}
	}

	//delete pTGAfile;
	fclose(pFile);
}
