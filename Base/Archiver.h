#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <zlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "MemoryManager.h"
#include "SpinLock.h"

class Archiver
{	
	static constexpr char PACKAGE_FILE_EXTENSION[] = ".txt";
	static constexpr char COMPRESSION_LEVEL = Z_DEFAULT_COMPRESSION;

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
			this->typeHash = 0;
			this->offset = 0;
			this->uncompressedSize = 0;
			this->compressedSize = 0;
		}

		PackageEntryDescriptor(size_t typeHash, size_t offset, size_t uncompressedSize, size_t compressedSize)
		{
			this->typeHash = typeHash;
			this->offset = offset;
			this->uncompressedSize = uncompressedSize;
			this->compressedSize = compressedSize;
		}

		size_t typeHash;
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

		UncompressedPackageEntry(size_t typeHash, size_t uncompressedSize, size_t compressedSize, void* pData)
		{
			this->packageEntryDesc = PackageEntryDescriptor(typeHash, 0, uncompressedSize, compressedSize);
			this->pData = pData;
		}

		UncompressedPackageEntry(size_t typeHash, size_t offset, size_t uncompressedSize, size_t compressedSize, void* pData)
		{
			this->packageEntryDesc = PackageEntryDescriptor(typeHash, offset, uncompressedSize, compressedSize);
			this->pData = pData;
		}

		~UncompressedPackageEntry()
		{
			if (pData != nullptr)
			{
				//MemoryManager::GetInstance().Free(pData);
				//pData = nullptr;
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
			this->pFileStream = nullptr;
			this->isPackageOpen = false;
			this->packageMode = UNDEFINED;
			this->pData = nullptr;
		}

		~Package()
		{
			Reset();
		}

		void Reset()
		{
			this->filename = "None";
			if (this->pFileStream != nullptr)
			{
				if (this->pFileStream->is_open())
					this->pFileStream->close();

				delete this->pFileStream;
			}
			this->pFileStream = nullptr;
			this->isPackageOpen = false;
			this->packageMode = UNDEFINED;
			this->table.clear();
		}

		std::string filename;
		std::ifstream* pFileStream;
		bool isPackageOpen;
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
	void CloseCompressedPackage();

	size_t ReadRequiredSizeForPackageData(size_t hash);
	bool ReadPackageData(size_t hash, size_t& typeHash, void* pBuf, size_t bufSize);

	void CreateUncompressedPackage();
	void AddToUncompressedPackage(size_t hash, size_t typeHash, size_t sizeInBytes, void* pData);
	void RemoveFromUncompressedPackage(size_t hash);
	void SaveUncompressedPackage(const std::string& filename);
	void CloseUncompressedPackage();

	void* DecompressHeader(std::ifstream& fileStream);
	size_t CompressHeader(void* pHeader, size_t headerSize, void* pBuf, size_t bufSize);

private:
	Archiver();
	size_t ReadPackageHeader(std::ifstream& fileStream);

private:
	Package m_CompressedPackage;
	std::map<size_t, UncompressedPackageEntry> m_UncompressedPackageEntries;

	SpinLock m_OpenPackageLock;
	SpinLock m_FileStreamLock;

public:
	static Archiver& GetInstance()
	{
		static Archiver archiver;
		return archiver;
	}
};

#endif