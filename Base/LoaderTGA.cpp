#include "LoaderTGA.h"
#include "Helpers.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"

#define INVERTED_BIT (1 << 5)
#define BYTE32 32
#define TGADEBUG

LoaderTGA::LoaderTGA()
{
}

LoaderTGA::~LoaderTGA()
{
}

IResource* LoaderTGA::LoadFromDisk(const std::string& file)
{

	TGAHeader pTGAfile;
	ReadFromDisk(file, pTGAfile);
	Texture* pTexture = new Texture(pTGAfile.imageWidth, pTGAfile.imageHeight, pTGAfile.imageDataBuffer);
	stack_delete(pTGAfile.imageDataBuffer);
	return pTexture;
}

IResource* LoaderTGA::LoadFromMemory(void* data, size_t guid)
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
	stack_delete(pTGAfile.imageDataBuffer);

	return sizeInBytes;
}

void LoaderTGA::ReadFromDisk(const std::string& file, TGAHeader& pTGAfile)
{
	FILE* pFile = nullptr;
	long imageSize;
	int colorMode;

	stack_reset();

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
		std::printf("Unsupported Compressed image type");
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
	if (pTGAfile.bitCount != 24 && pTGAfile.bitCount != 32)
	{
		std::printf("Unsupported number of bits per Pixel");
		debugbreak();
	}
	fread(&pTGAfile.imageDescr, sizeof(unsigned char), 1, pFile);

	//field 6 (byte length depending on IDLengths, max 255)
	char buffer[255];
	fread(buffer, sizeof(unsigned char), pTGAfile.idLength, pFile);


	//For a Pixel Depth of 15 and 16 bit, 
	//each pixel is stored with 5 bits per color. 
	//If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
	// Color mode -> 3 = BGR, 4 = BGRA. Need to perform a color swap
	//color mode has to be 4, sf::Texture doesnt support 24 birt
	colorMode = pTGAfile.bitCount / 8;
	imageSize = pTGAfile.imageWidth * pTGAfile.imageHeight * colorMode;

	//allocating memory for the data
	//pTGAfile.imageDataBuffer = (unsigned char*)malloc((sizeof(unsigned char)*imageSize));
	pTGAfile.imageDataBuffer = (unsigned char*)stack_allocate((sizeof(unsigned char)*imageSize), 1, "TextureTGA");

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
#if defined(TGADEBUG)
	std::cout << "------------------------------------------" << std::endl;
	std::cout << "file '" + file + "' header data:" << std::endl;

	if (pTGAfile.imageType == 2)
	{
		std::cout << "image type(2): uncompressed true-color image." << std::endl;
	}
	else if (pTGAfile.imageType == 10)
	{
		std::cout << "image type(10): run-length encoded true-color image" << std::endl;
	}
	std::cout << "pixel start x: " + std::to_string(pTGAfile.startX) << std::endl;
	std::cout << "pixel start y: " + std::to_string(pTGAfile.startX) << std::endl;
	std::cout << "image width: " + std::to_string(pTGAfile.imageWidth) << std::endl;
	std::cout << "image height: " + std::to_string(pTGAfile.imageHeight) << std::endl;
	std::cout << "pixel size(in bits): " + std::to_string(pTGAfile.bitCount) << std::endl;
	std::cout << "image size: " + std::to_string(imageSize) << std::endl;
	if (!(pTGAfile.imageDescr & INVERTED_BIT))
	{
		std::cout << "pixel order: top-to-bottom"<< std::endl;
	}
	else
	{
		std::cout << "pixel order: left-to-right" << std::endl;
	}
	std::cout << "------------------------------------------" << std::endl;
#endif
	fclose(pFile);
}
