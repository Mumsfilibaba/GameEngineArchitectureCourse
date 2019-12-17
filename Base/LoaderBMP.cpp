#include "LoaderBMP.h"
#include "StackAllocator.h"

LoaderBMP::LoaderBMP()
{

}

LoaderBMP::~LoaderBMP()
{
}

IResource* LoaderBMP::LoadFromDisk(const std::string& file)
{
	std::ifstream fileStream;
	fileStream.open(file, std::ios_base::in | std::ios_base::binary);

	fileStream.seekg(0, std::ios::end);
	size_t sizeInBytes = fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);

	void* pBuf = stack_allocate(sizeInBytes, 1, "BMP Buffer");
	fileStream.read(reinterpret_cast<char*>(pBuf), sizeInBytes);

	size_t startAddress = (size_t)pBuf;
	BMPHeader bmpHeader;
	DIBHeader_BITMAPINFOHEADER_40 dibHeader;

	memcpy(&bmpHeader, pBuf, sizeof(BMPHeader));
	memcpy(&dibHeader, (void*)(startAddress + 14), 40);

	switch (dibHeader.dibHeaderSize)
	{
	case BITMAPINFOHEADER_40:
		std::cout << "BMP DIB Header: BITMAPINFOHEADER_40" << std::endl;
		break;
	case OS21XBITMAPHEADER_12:
		std::cout << "Unsupported BMP DIB Header: OS21XBITMAPHEADER_12" << std::endl;
		return 0;
		break;
	case OS22XBITMAPHEADER_64:
		std::cout << "Unsupported BMP DIB Header: OS22XBITMAPHEADER_64" << std::endl;
		return 0;
		break;
	case OS22XBITMAPHEADER_16:
		std::cout << "Unsupported BMP DIB Header: OS22XBITMAPHEADER_16" << std::endl;
		return 0;
		break;
	case BITMAPV2INFOHEADER_52:
		std::cout << "Unsupported BMP DIB Header: BITMAPV2INFOHEADER_52" << std::endl;
		return 0;
		break;
	case BITMAPV3INFOHEADER_56:
		std::cout << "Unsupported BMP DIB Header: BITMAPV3INFOHEADER_56" << std::endl;
		return 0;
		break;
	case BITMAPV4HEADER_108:
		std::cout << "Unsupported BMP DIB Header: BITMAPV4HEADER_108" << std::endl;
		return 0;
		break;
	case BITMAPV5HEADER_124:
		std::cout << "Unsupported BMP DIB Header: BITMAPV5HEADER_124" << std::endl;
		return 0;
		break;
	default:
		std::cout << "Undefined BMP Format Detected!" << std::endl;
		return 0;
		break;
	}

	if (dibHeader.compressionMethod != 0)
	{
		std::cout << "Unsupported Compression Method on BMP" << std::endl;
		return 0;
	}

	if (dibHeader.numBitsPerPixel != 24)
	{
		std::cout << "Unsopported Number of Bits per Pixel on BMP" << std::endl;
		return 0;
	}

	size_t totalPixelDataSize = dibHeader.height * (size_t)ceilf(dibHeader.numBitsPerPixel * dibHeader.width / 32.0f) * 4;

	void* pPixelData = stack_allocate(totalPixelDataSize, 1, "BMP Pixel Data");
	size_t pixelDataStartAddress = (size_t)pPixelData;
	memcpy(pPixelData, (void*)(startAddress + bmpHeader.pixelDataOffset), totalPixelDataSize);
	//MemoryManager::GetInstance().Free(pPixelData);

	size_t pixelDataLength = dibHeader.width * dibHeader.height;
	BMPPixel* pBMPConvertedPixels = (BMPPixel*)stack_allocate(pixelDataLength * sizeof(BMPPixel), 1, "BMP Converted Pixel Data");

	for (size_t i = 0; i < pixelDataLength; i++)
	{
		char bgr[3];
		memcpy(bgr, (void*)(pixelDataStartAddress + i * 3), 3);

		pBMPConvertedPixels[i].r = bgr[2];
		pBMPConvertedPixels[i].g = bgr[1];
		pBMPConvertedPixels[i].b = bgr[0];
		pBMPConvertedPixels[i].a = 255;
	}

	Texture* pTexture = new(file.c_str()) Texture(dibHeader.width, dibHeader.height, reinterpret_cast<unsigned char*>(pBMPConvertedPixels));
	stack_reset();

	return pTexture;
}

IResource* LoaderBMP::LoadFromMemory(void* pData, size_t)
{
	size_t dataStartAddress = (size_t)pData;
	unsigned int width;
	unsigned int height;

	memcpy(&width, pData, sizeof(width));
	memcpy(&height, (void*)(dataStartAddress + sizeof(width)), sizeof(height));

	size_t pixelDataSize = (size_t)width * (size_t)height * sizeof(BMPPixel);

	void* pPixelData = stack_allocate(pixelDataSize, 1, "BMP Texture Pixel Data");
	memcpy(pPixelData, (void*)(dataStartAddress + sizeof(width) + sizeof(height)), pixelDataSize);

	Texture* pTexture = new("Texture Loaded From Memory") Texture(width, height, reinterpret_cast<unsigned char*>(pPixelData));
	stack_reset();

	return pTexture;
}

size_t LoaderBMP::WriteToBuffer(const std::string& file, void* pBuffer)
{
	std::ifstream fileStream;
	fileStream.open(file, std::ios_base::in | std::ios_base::binary);

	fileStream.seekg(0, std::ios::end);
	size_t sizeInBytes = fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);

	void* pBMPFileData = stack_allocate(sizeInBytes, 1, "BMP File Data");
	fileStream.read(reinterpret_cast<char*>(pBMPFileData), sizeInBytes);

	size_t textureSize = LoadAndConvert(pBMPFileData, sizeInBytes, pBuffer);
	//MemoryManager::GetInstance().Free(pBMPFileData);
	stack_reset();
	return textureSize;
}

size_t LoaderBMP::LoadAndConvert(void* pBMPFileData, size_t, void* pBuffer)
{
	size_t startAddress = (size_t)pBMPFileData;
	BMPHeader bmpHeader;
	DIBHeader_BITMAPINFOHEADER_40 dibHeader;

	memcpy(&bmpHeader, pBMPFileData, sizeof(BMPHeader));
	memcpy(&dibHeader, (void*)(startAddress + 14), 40);

	switch (dibHeader.dibHeaderSize)
	{
	case BITMAPINFOHEADER_40:
		std::cout << "BMP DIB Header: BITMAPINFOHEADER_40" << std::endl;
		break;
	case OS21XBITMAPHEADER_12:
		std::cout << "Unsupported BMP DIB Header: OS21XBITMAPHEADER_12" << std::endl;
		return 0;
		break;
	case OS22XBITMAPHEADER_64:
		std::cout << "Unsupported BMP DIB Header: OS22XBITMAPHEADER_64" << std::endl;
		return 0;
		break;
	case OS22XBITMAPHEADER_16:
		std::cout << "Unsupported BMP DIB Header: OS22XBITMAPHEADER_16" << std::endl;
		return 0;
		break;
	case BITMAPV2INFOHEADER_52:
		std::cout << "Unsupported BMP DIB Header: BITMAPV2INFOHEADER_52" << std::endl;
		return 0;
		break;
	case BITMAPV3INFOHEADER_56:
		std::cout << "Unsupported BMP DIB Header: BITMAPV3INFOHEADER_56" << std::endl;
		return 0;
		break;
	case BITMAPV4HEADER_108:
		std::cout << "Unsupported BMP DIB Header: BITMAPV4HEADER_108" << std::endl;
		return 0;
		break;
	case BITMAPV5HEADER_124:
		std::cout << "Unsupported BMP DIB Header: BITMAPV5HEADER_124" << std::endl;
		return 0;
		break;
	default:
		std::cout << "Undefined BMP Format Detected!" << std::endl;
		return 0;
		break;
	}

	if (dibHeader.compressionMethod != 0)
	{
		std::cout << "Unsupported Compression Method on BMP" << std::endl;
		return 0;
	}

	if (dibHeader.numBitsPerPixel != 24)
	{
		std::cout << "Unsupported Number of Bits per Pixel on BMP" << std::endl;
		return 0;
	}

	size_t totalPixelDataSize = dibHeader.height * (size_t)ceilf(dibHeader.numBitsPerPixel * dibHeader.width / 32.0f) * 4;

	void* pPixelData = stack_allocate(totalPixelDataSize, 1, "BMP Pixel Data");
	size_t pixelDataStartAddress = (size_t)pPixelData;
	memcpy(pPixelData, (void*)(startAddress + bmpHeader.pixelDataOffset), totalPixelDataSize);
	//MemoryManager::GetInstance().Free(pPixelData);

	size_t pixelDataLength = dibHeader.width * dibHeader.height;
	BMPPixel* pBMPConvertedPixels = (BMPPixel*)stack_allocate(pixelDataLength * sizeof(BMPPixel), 1, "BMP Converted Pixel Data");

	for (size_t i = 0; i < pixelDataLength; i++)
	{
		char bgr[3];
		memcpy(bgr, (void*)(pixelDataStartAddress + i * 3), 3);

		pBMPConvertedPixels[i].r = bgr[2];
		pBMPConvertedPixels[i].g = bgr[1];
		pBMPConvertedPixels[i].b = bgr[0];
		pBMPConvertedPixels[i].a = 255;
	}

	size_t bufferStartAddress = (size_t)pBuffer;
	memcpy(pBuffer, &dibHeader.width, sizeof(dibHeader.width));
	memcpy((void*)(bufferStartAddress + sizeof(dibHeader.width)), &dibHeader.height, sizeof(dibHeader.height));
	memcpy((void*)(bufferStartAddress + sizeof(dibHeader.width) + sizeof(dibHeader.height)), pBMPConvertedPixels, sizeof(BMPPixel) * pixelDataLength);
	//MemoryManager::GetInstance().Free(pBMPConvertedPixels);

	stack_reset();
	return sizeof(dibHeader.width) + sizeof(dibHeader.height) + sizeof(BMPPixel) * pixelDataLength;
}
