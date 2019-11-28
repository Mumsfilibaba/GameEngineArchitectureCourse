#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <zlib.h>
#include <fstream>
#include <iostream>

#include "MemoryManager.h"
#include "StringHash.h"

class Archiver
{
	struct PackageTableData
	{
		PackageTableData()
		{
			this->offset = 0;
			this->uncompressedSize = 0;
			this->compressedSize = 0;
		}

		PackageTableData(size_t offset, size_t uncompressedSize, size_t compressedSize)
		{
			this->offset = offset;
			this->uncompressedSize = uncompressedSize;
			this->compressedSize = compressedSize;
		}

		size_t offset;
		size_t uncompressedSize;
		size_t compressedSize;
	};

	struct Package
	{
		Package()
		{
			this->filename = "None";
			this->isOpen = false;
			this->pData = nullptr;
		}

		void Reset()
		{
			this->filename = "None";
			this->isOpen = false;
			this->table.clear();
		}

		std::string filename;
		bool isOpen;
		std::unordered_map<size_t, PackageTableData> table;
		void* pData;
	};

	struct PackageEntry
	{
		PackageEntry()
		{
			this->offset = 0;
			this->uncompressedSize = 0;
			this->compressedSize = 0;
			this->pData = nullptr;
		}

		PackageEntry(size_t uncompressedSize, size_t compressedSize, void* pData)
		{
			this->uncompressedSize = uncompressedSize;
			this->compressedSize = compressedSize;
			this->pData = pData;
		}

		PackageEntry(size_t offset, size_t uncompressedSize, size_t compressedSize, void* pData)
		{
			this->offset = offset;
			this->uncompressedSize = uncompressedSize;
			this->compressedSize = compressedSize;
			this->pData = pData;
		}

		~PackageEntry()
		{
			if (pData != nullptr)
			{
				MemoryManager::GetInstance().Free(pData);
				pData = nullptr;
			}
		}

		size_t offset;
		size_t uncompressedSize;
		size_t compressedSize;
		void* pData;
	};

public:
	~Archiver();

	void OpenCompressedPackage(const std::string& filename);
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
	std::map<size_t, PackageEntry> m_UncompressedPackageEntries;

public:
	static Archiver& GetInstance()
	{
		static Archiver archiver;
		return archiver;
	}
};

#endif