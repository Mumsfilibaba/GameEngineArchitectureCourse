#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <fstream>
#include <iostream>

class TextureManager
{
	struct TGAFile
	{
		//// 0–255 The number of bytes that the image ID field consists of.
		//unsigned char imageIDLength;

		////color-map type
		//unsigned char imageColorMapType;

		////image type
		//unsigned char imageType;

		////Color-map Specs
		//unsigned short imageColorMapStart;
		//unsigned short imageColorMapLength;
		//unsigned char imageColorMapSize;

		////image specifications
		//unsigned short imageX; //x-Origin
		//unsigned short imageY; //y-Origin
		//unsigned short imageWidth;//in pixels
		//unsigned short imageHeight;//in pixels
		//unsigned char pixelDepth;
		//unsigned char imageDescriptor;

		unsigned char imageType;
		short int imageWidth;
		short int imageHeight;
		unsigned char pixelDepth;

		unsigned char *imageDataBuffer;
	};
public:

	TGAFile m_tgaFile;

private:


public:
	//maybe make it return the TGAFile-pointer
	void LoadTGAFile(char* fileName, TGAFile file);

	TextureManager() = default;
	~TextureManager() = default;


};


#endif // !TEXTUREMANAGER_H
