#ifndef LOADER_BMP_H
#define LOADER_BMP_H

#include <fstream>
#include <iostream>

#include "ILoader.h"
#include "MemoryManager.h"
#include "Texture.h"

class LoaderBMP : public ILoader
{
	static constexpr unsigned int OS21XBITMAPHEADER_12 = 12;
	static constexpr unsigned int OS22XBITMAPHEADER_64 = 64;
	static constexpr unsigned int OS22XBITMAPHEADER_16 = 16;
	static constexpr unsigned int BITMAPINFOHEADER_40 = 40;
	static constexpr unsigned int BITMAPV2INFOHEADER_52 = 52;
	static constexpr unsigned int BITMAPV3INFOHEADER_56 = 56;
	static constexpr unsigned int BITMAPV4HEADER_108 = 108;
	static constexpr unsigned int BITMAPV5HEADER_124 = 124;

#pragma pack(push, 1)
	struct BMPHeader
	{
		unsigned short headerField;
		unsigned int bmpSize;
		unsigned short reserved1;
		unsigned short reserved2;
		unsigned int pixelDataOffset;
	};
#pragma pack(pop)

	struct DIBHeader_BITMAPINFOHEADER_40
	{
		unsigned int dibHeaderSize;
		unsigned int width;
		unsigned int height;
		unsigned short colorPlanes;
		unsigned short numBitsPerPixel;
		unsigned int compressionMethod;
		unsigned int bitmapDataSize;
		int horizontalResolution;
		int vericalResolution;
		unsigned int numColorsInPalette;
		unsigned int numImportantColors;
	};

#pragma pack(push, 1)
	struct BMPPixel
	{
		union
		{
			struct 
			{
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char a;
			};

			char channels[4];
		};
	};
#pragma pack(pop)

public:
	LoaderBMP();
	~LoaderBMP();

	virtual IResource* LoadFromDisk(const std::string& file) override;
	virtual IResource* LoadFromMemory(void* pData, size_t size) override;
	virtual size_t WriteToBuffer(const std::string& file, void* pBuffer) override;
    
private:
	size_t LoadAndConvert(void* pBMPFileData, size_t size, void* pBuffer);
};

#endif