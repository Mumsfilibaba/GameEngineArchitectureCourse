#include "Archiver.h"

#define ARCHIVER_CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        assert(false); \
    } \
}

void* ArchiverAlloc(void* pData, unsigned num, unsigned size)
{
	return MemoryManager::GetInstance().Allocate(num * size, 1, "Archiver");
}

void ArchiverFree(void* pData, void* pAddress)
{
	MemoryManager::GetInstance().Free(pAddress);
}

static alloc_func zalloc = ArchiverAlloc;
static free_func zfree = ArchiverFree;

Archiver::Archiver()
{
}

Archiver::~Archiver()
{
}

size_t Archiver::ReadPackageHeader(std::ifstream& fileStream)
{
	size_t tableEntries;
	size_t dataSize;
	fileStream >> tableEntries;
	fileStream >> dataSize;

	for (size_t i = 0; i < tableEntries; i++)
	{
		size_t hash;
		size_t offset;
		size_t uncompressedSize;
		size_t compressedSize;

		fileStream >> hash;
		fileStream >> offset;
		fileStream >> uncompressedSize;
		fileStream >> compressedSize;

		m_CompressedPackage.table[hash] = PackageEntryDescriptor(offset, uncompressedSize, compressedSize);
	}

	return dataSize;
}

void Archiver::OpenCompressedPackage(const std::string& filename, PackageMode packageMode)
{
	assert(!m_CompressedPackage.isPackageOpen);
	assert(packageMode != UNDEFINED);

	m_CompressedPackage.filename = filename + PACKAGE_FILE_EXTENSION;
	m_CompressedPackage.isPackageOpen = true;
	m_CompressedPackage.packageMode = packageMode;

	switch (packageMode)
	{
		case LOAD_AND_STORE:
		{
			std::ifstream fileStream;
			fileStream.open(filename + PACKAGE_FILE_EXTENSION, std::ios::in | std::ios::binary);

			if (fileStream.is_open())
			{
				size_t dataSize = ReadPackageHeader(fileStream);

				fileStream.seekg(1, std::ios_base::cur);
				m_CompressedPackage.pData = MemoryManager::GetInstance().Allocate(dataSize, 1, "Package Data");
				fileStream.read(reinterpret_cast<char*>(m_CompressedPackage.pData), dataSize);
				//std::cout << "Loaded Data: " << reinterpret_cast<char*>(m_CompressedPackage.pData) << std::endl;
				fileStream.close();
			}

			break;
		}
		case LOAD_AND_PREPARE:
		{
			m_CompressedPackage.pFileStream = new std::ifstream();
			m_CompressedPackage.pFileStream->open(filename + PACKAGE_FILE_EXTENSION, std::ios::in | std::ios::binary);

			std::ifstream& fileStream = *m_CompressedPackage.pFileStream;

			if (fileStream.is_open())
			{
				ReadPackageHeader(fileStream);

				fileStream.seekg(1, std::ios_base::cur);
				m_CompressedPackage.fileDataStart = fileStream.tellg();
				m_CompressedPackage.pFileStream = &fileStream;
				fileStream.close();
			}
			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}
}

void Archiver::CloseCompressedPackage()
{
	assert(m_CompressedPackage.isPackageOpen);
	assert(!m_CompressedPackage.pFileStream->is_open());

	ClosePackageForReading();
	m_CompressedPackage.Reset();
}

size_t Archiver::ReadRequiredSizeForPackageData(size_t hash)
{
	auto packageTableEntry = m_CompressedPackage.table.find(hash);
	if (packageTableEntry == m_CompressedPackage.table.end())
		return 0;

	return packageTableEntry->second.uncompressedSize;
}

void Archiver::OpenPackageForReading()
{
	if (m_CompressedPackage.pFileStream != nullptr)
	{
		if (!m_CompressedPackage.pFileStream->is_open())
			m_CompressedPackage.pFileStream->open(m_CompressedPackage.filename, std::ios::in | std::ios::binary);
	}
}

void Archiver::ClosePackageForReading()
{
	if (m_CompressedPackage.pFileStream != nullptr)
	{
		if (m_CompressedPackage.pFileStream->is_open())
			m_CompressedPackage.pFileStream->close();
	}
}

void Archiver::ReadPackageData(size_t hash, void* pBuf, size_t bufSize)
{
	auto packageTableEntry = m_CompressedPackage.table.find(hash);
	if(packageTableEntry == m_CompressedPackage.table.end())
		return;

	if (bufSize < packageTableEntry->second.uncompressedSize)
		return;

	void* pCompressedStart = nullptr;

	switch (m_CompressedPackage.packageMode)
	{
		case LOAD_AND_STORE:
		{
			pCompressedStart = reinterpret_cast<void*>((size_t)m_CompressedPackage.pData + packageTableEntry->second.offset);
			break;
		}
		case LOAD_AND_PREPARE:
		{
			assert(m_CompressedPackage.pFileStream != nullptr);

			std::ifstream& fileStream = *m_CompressedPackage.pFileStream;
			if (m_CompressedPackage.pFileStream->is_open())
			{
				fileStream.seekg(m_CompressedPackage.fileDataStart + packageTableEntry->second.offset, std::ios_base::beg);

				if (packageTableEntry->second.compressedSize > 0)
				{
					pCompressedStart = MemoryManager::GetInstance().Allocate(packageTableEntry->second.compressedSize, 1, "Package Data");
					fileStream.read(reinterpret_cast<char*>(pCompressedStart), packageTableEntry->second.compressedSize);
				}
				else
				{
					fileStream.read(reinterpret_cast<char*>(pBuf), packageTableEntry->second.uncompressedSize);

					return;
				}
				
			}

			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}

	if (packageTableEntry->second.compressedSize > 0)
	{
		int err;
		z_stream decompressionStream;
		decompressionStream.zalloc = zalloc;
		decompressionStream.zfree = zfree;
		decompressionStream.opaque = nullptr;

		err = inflateInit(&decompressionStream);
		ARCHIVER_CHECK_ERR(err, "inflateInit");

		//std::cout << "Loaded Data: " << reinterpret_cast<char*>(pCompressedStart) << std::endl;

		decompressionStream.next_in = reinterpret_cast<Byte*>(pCompressedStart);
		decompressionStream.next_out = reinterpret_cast<Byte*>(pBuf);
		decompressionStream.avail_in = packageTableEntry->second.compressedSize;
		decompressionStream.avail_out = packageTableEntry->second.uncompressedSize;

		err = inflate(&decompressionStream, Z_FINISH);
		assert(err > Z_OK);

		err = inflateEnd(&decompressionStream);
		ARCHIVER_CHECK_ERR(err, "inflateEnd");
	}
	else
	{
		memcpy(pCompressedStart, pBuf, packageTableEntry->second.uncompressedSize);
	}

	if (m_CompressedPackage.packageMode == LOAD_AND_PREPARE)
		MemoryManager::GetInstance().Free(pCompressedStart);
}

void Archiver::CreateUncompressedPackage()
{
	m_UncompressedPackageEntries.clear();
}

void Archiver::AddToUncompressedPackage(size_t hash, size_t sizeInBytes, void* pData)
{
	void* pDataCopy = MemoryManager::GetInstance().Allocate(sizeInBytes, 1, "Uncompressed Package Data");
	memcpy(pDataCopy, pData, sizeInBytes);
	m_UncompressedPackageEntries[hash] = UncompressedPackageEntry(sizeInBytes, 0, pDataCopy);
}

void Archiver::RemoveFromUncompressedPackage(size_t hash)
{
	m_UncompressedPackageEntries.erase(hash);
}

void Archiver::SaveUncompressedPackage(const std::string& filename)
{
	std::unordered_map<size_t, UncompressedPackageEntry> compressedPackageEntries;

#ifdef _DEBUG
	size_t uncompressedDataSize = 0;
#endif

	size_t compressedDataSize = 0;

	for (auto& it : m_UncompressedPackageEntries)
	{
		int err;
		z_stream compressionStream;
		compressionStream.zalloc = zalloc;
		compressionStream.zfree = zfree;
		compressionStream.opaque = nullptr;

		err = deflateInit(&compressionStream, Z_DEFAULT_COMPRESSION);
		//err = deflateInit(&compressionStream, Z_BEST_COMPRESSION);
		ARCHIVER_CHECK_ERR(err, "deflateInit");
		
		void* pCompressed = MemoryManager::GetInstance().Allocate(it.second.packageEntryDesc.uncompressedSize, 1, "Archiver Compressed Buffer");
		compressionStream.next_in = reinterpret_cast<Byte*>(it.second.pData);
		compressionStream.next_out = reinterpret_cast<Byte*>(pCompressed);
		compressionStream.avail_in = it.second.packageEntryDesc.uncompressedSize;
		compressionStream.avail_out = it.second.packageEntryDesc.uncompressedSize;

		err = deflate(&compressionStream, Z_FINISH);

		if (err == Z_OK || err == Z_BUF_ERROR)
		{
			//Compressed Size is larger than Uncompressed Size
			err = deflateEnd(&compressionStream);
			ARCHIVER_CHECK_ERR(err, "deflateEnd");

			memcpy(pCompressed, it.second.pData, it.second.packageEntryDesc.uncompressedSize);
			compressedPackageEntries[it.first] = UncompressedPackageEntry(compressedDataSize, it.second.packageEntryDesc.uncompressedSize, 0, pCompressed);
			compressedDataSize += it.second.packageEntryDesc.uncompressedSize;
		}
		else if (err == Z_STREAM_END)
		{
			err = deflateEnd(&compressionStream);
			ARCHIVER_CHECK_ERR(err, "deflateEnd");

			compressedPackageEntries[it.first] = UncompressedPackageEntry(compressedDataSize, 
				it.second.packageEntryDesc.uncompressedSize, 
				compressionStream.total_out, pCompressed);
			compressedDataSize += compressionStream.total_out;
		}

#ifdef _DEBUG
		uncompressedDataSize += it.second.packageEntryDesc.uncompressedSize;
#endif

		MemoryManager::GetInstance().Free(pCompressed);
	}

	std::ofstream file;
	file.open(filename + PACKAGE_FILE_EXTENSION, std::ios::out | std::ios::trunc | std::ios::binary);

	file << compressedPackageEntries.size() << std::endl;
	file << compressedDataSize << std::endl;

	for (auto& it : compressedPackageEntries)
	{
		file << it.first << std::endl;
		file << it.second.packageEntryDesc.offset << std::endl;
		file << it.second.packageEntryDesc.uncompressedSize << std::endl;
		file << it.second.packageEntryDesc.compressedSize << std::endl;
	}

	for (auto& it : compressedPackageEntries)
	{
		if (it.second.packageEntryDesc.compressedSize > 0)
			file.write(reinterpret_cast<char*>(it.second.pData), it.second.packageEntryDesc.compressedSize);
		else
			file.write(reinterpret_cast<char*>(it.second.pData), it.second.packageEntryDesc.uncompressedSize);
	}

	file.close();

#ifdef _DEBUG
	std::cout << std::endl << "Compressed Package: " << std::endl;
	std::cout << "Uncompressed Size: " << uncompressedDataSize << " bytes" << std::endl;
	std::cout << "Compressed Size: " << compressedDataSize << " bytes" << std::endl;
	std::cout << "Compressed Relative Size: " << (100.0f * (float)compressedDataSize / uncompressedDataSize) << "%" << std::endl << std::endl;

	{
		std::ofstream debugFile;
		debugFile.open(filename + "_debug" + PACKAGE_FILE_EXTENSION, std::ios::out | std::ios::trunc);

		debugFile << "Table Enties: " << compressedPackageEntries.size() << std::endl;
		debugFile << "Data Size: " << compressedDataSize << std::endl;

		for (auto& it : compressedPackageEntries)
		{
			debugFile << "Hash: " << it.first << std::endl;
			debugFile << "Offset: " << it.second.packageEntryDesc.offset << std::endl;
			debugFile << "UncompressedSize: " << it.second.packageEntryDesc.uncompressedSize << std::endl;
			debugFile << "CompressedSize: " << it.second.packageEntryDesc.compressedSize << std::endl;
		}

		debugFile << "Data: ";
		for (auto& it : compressedPackageEntries)
		{
			debugFile.write(reinterpret_cast<char*>(it.second.pData), it.second.packageEntryDesc.compressedSize);
		}

		debugFile.close();
	}
#endif
}

void Archiver::CloseUncompressedPackage()
{
	for (auto& it : m_UncompressedPackageEntries)
	{
		MemoryManager::GetInstance().Free(it.second.pData);
		it.second.pData = nullptr;
	}

	m_UncompressedPackageEntries.clear();
}
