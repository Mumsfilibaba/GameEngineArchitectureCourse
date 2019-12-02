#include "LoaderBMP.h"

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

	void* pBuf = MemoryManager::GetInstance().Allocate(sizeInBytes, 1, "BMP Buffer");
	fileStream.read(reinterpret_cast<char*>(pBuf), sizeInBytes);

	return LoadFromMemory(pBuf, sizeInBytes);
}

IResource* LoaderBMP::LoadFromMemory(void* pData, size_t size)
{
	size_t startAddress = (size_t)pData;
	BMPHeader bmpHeader;
	DIBHeader_BITMAPINFOHEADER_40 dibHeader;

	memcpy(&bmpHeader, pData, sizeof(BMPHeader));
	memcpy(&dibHeader, (void*)(startAddress + 14), 40);

	switch (dibHeader.dibHeaderSize)
	{
	case BITMAPINFOHEADER_40:
		std::cout << "BMP DIF Header: BITMAPINFOHEADER_40" << std::endl;
		break;
	case OS21XBITMAPHEADER_12:
		std::cout << "Unsupported BMP DIF Header: OS21XBITMAPHEADER_12" << std::endl;
		break;
	case OS22XBITMAPHEADER_64:
		std::cout << "Unsupported BMP DIF Header: OS22XBITMAPHEADER_64" << std::endl;
		break;
	case OS22XBITMAPHEADER_16:
		std::cout << "Unsupported BMP DIF Header: OS22XBITMAPHEADER_16" << std::endl;
		break;
	case BITMAPV2INFOHEADER_52:
		std::cout << "Unsupported BMP DIF Header: BITMAPV2INFOHEADER_52" << std::endl;
		break;
	case BITMAPV3INFOHEADER_56:
		std::cout << "Unsupported BMP DIF Header: BITMAPV3INFOHEADER_56" << std::endl;
		break;
	case BITMAPV4HEADER_108:
		std::cout << "Unsupported BMP DIF Header: BITMAPV4HEADER_108" << std::endl;
		break;
	case BITMAPV5HEADER_124:
		std::cout << "Unsupported BMP DIF Header: BITMAPV5HEADER_124" << std::endl;
		break;
	default:
		std::cout << "Undefined BMP Format Detected!" << std::endl;
		break;
	}

	if (dibHeader.compressionMethod != 0)
	{
		std::cout << "Unsupported Compression Method on BMP" << std::endl;
	}

	size_t totalPixelDataSize = dibHeader.height * (size_t)ceilf(dibHeader.numBitsPerPixel * dibHeader.width / 32.0f) * 4;

	void* pPixelData = MemoryManager::GetInstance().Allocate(totalPixelDataSize, 1, "BMP Pixel Data");
	size_t pixelDataStartAddress = (size_t)pPixelData;
	memcpy(pPixelData, (void*)(startAddress + bmpHeader.pixelDataOffset), totalPixelDataSize);

	size_t pixelDataLength = dibHeader.width * dibHeader.height;
	BMPPixel* pBMPConvertedPixels = (BMPPixel*)MemoryManager::GetInstance().Allocate(pixelDataLength * 4, 1, "BMP Converted Pixel Data");

	for (size_t i = 0; i < pixelDataLength; i++)
	{
		char bgr[3];
		memcpy(bgr, (void*)(pixelDataStartAddress + i * 3), 3);

		pBMPConvertedPixels[i].r = bgr[2];
		pBMPConvertedPixels[i].g = bgr[1];
		pBMPConvertedPixels[i].b = bgr[0];
		pBMPConvertedPixels[i].a = 255;
	}

	return new Texture(dibHeader.width, dibHeader.height, reinterpret_cast<unsigned char*>(pBMPConvertedPixels));
}

size_t LoaderBMP::WriteToBuffer(const std::string& file, void* pBuffer)
{
	std::ifstream fileStream;
	fileStream.open(file, std::ios_base::in | std::ios_base::binary);

	fileStream.seekg(0, std::ios::end);
	size_t sizeInBytes = fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);

	fileStream.read(reinterpret_cast<char*>(pBuffer), sizeInBytes);

	return sizeInBytes;
}