#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <zlib.h>
#include <fstream>
#include <iostream>

#include "MemoryManager.h"
#include "StringHash.h"

class Archiver
{	
	static constexpr char PACKAGE_FILE_EXTENSION[] = ".txt";

public:
	enum PackageMode : unsigned char
	{
		UNDEFINED,
		LOAD_AND_STORE,
		LOAD_AND_PREPARE
	};

private:
	struct PackageEntryDescriptor
	{
		PackageEntryDescriptor()
		{
			this->offset = 0;
			this->uncompressedSize = 0;
			this->compressedSize = 0;
		}

		PackageEntryDescriptor(size_t offset, size_t uncompressedSize, size_t compressedSize)
		{
			this->offset = offset;
			this->uncompressedSize = uncompressedSize;
			this->compressedSize = compressedSize;
		}

		size_t offset;
		size_t uncompressedSize;
		size_t compressedSize;
	};

	struct UncompressedPackageEntry
	{
		UncompressedPackageEntry()
		{
			this->pData = nullptr;
		}

		UncompressedPackageEntry(size_t uncompressedSize, size_t compressedSize, void* pData)
		{
			this->packageEntryDesc = PackageEntryDescriptor(0, uncompressedSize, compressedSize);
			this->pData = pData;
		}

		UncompressedPackageEntry(size_t offset, size_t uncompressedSize, size_t compressedSize, void* pData)
		{
			this->packageEntryDesc = PackageEntryDescriptor(offset, uncompressedSize, compressedSize);
			this->pData = pData;
		}

		~UncompressedPackageEntry()
		{
			if (pData != nullptr)
			{
				MemoryManager::GetInstance().Free(pData);
				pData = nullptr;
			}
		}

		PackageEntryDescriptor packageEntryDesc;
		void* pData;
	};

	struct Package
	{
		Package()
		{
			this->filename = "None";
			this->isOpen = false;
			this->packageMode = UNDEFINED;
			this->pData = nullptr;
		}

		void Reset()
		{
			this->filename = "None";
			this->isOpen = false;
			this->packageMode = UNDEFINED;
			this->table.clear();
		}

		std::string filename;
		bool isOpen;
		PackageMode packageMode;
		std::unordered_map<size_t, PackageEntryDescriptor> table;

		union
		{
			void* pData;
			size_t fileDataStart;
		};
	};

public:
	~Archiver();

	void OpenCompressedPackage(const std::string& filename, PackageMode packageMode);
	size_t ReadRequiredSizeForPackageData(size_t hash);
	void ReadPackageData(size_t hash, void* pBuf, size_t bufSize);
	void CloseCompressedPackage();

	void CreateUncompressedPackage();
	void AddToUncompressedPackage(size_t hash, size_t sizeInBytes, void* pData);
	void RemoveFromUncompressedPackage(size_t hash);
	void SaveUncompressedPackage(const std::string& filename);
	void CloseUncompressedPackage();

private:
	Archiver();

private:
	Package m_CompressedPackage;
	std::map<size_t, UncompressedPackageEntry> m_UncompressedPackageEntries;

public:
	static Archiver& GetInstance()
	{
		static Archiver archiver;
		return archiver;
	}
};

#endif