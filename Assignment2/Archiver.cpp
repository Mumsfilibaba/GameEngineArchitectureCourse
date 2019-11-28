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

void Archiver::OpenCompressedPackage(const std::string& filename)
{
	assert(!m_CompressedPackage.isOpen);

	m_CompressedPackage.filename = filename;
	m_CompressedPackage.isOpen = true;

	std::ifstream file;
	file.open(filename + ".chat", std::ios::in | std::ios::binary);

	size_t tableEntries;
	size_t dataSize;
	file >> tableEntries;
	file >> dataSize;

	for (size_t i = 0; i < tableEntries; i++)
	{
		size_t hash;
		size_t offset;
		size_t uncompressedSize;
		size_t compressedSize;

		file >> hash;
		file >> offset;
		file >> uncompressedSize;
		file >> compressedSize;

		m_CompressedPackage.table[hash] = PackageTableData(offset, uncompressedSize, compressedSize);
	}

	file.seekg(1, std::ios_base::cur);
	m_CompressedPackage.pData = MemoryManager::GetInstance().Allocate(dataSize, 1, "Package Data");
	file.read(reinterpret_cast<char*>(m_CompressedPackage.pData), dataSize);
	//std::cout << "Loaded Data: " << reinterpret_cast<char*>(m_CompressedPackage.pData) << std::endl;

	file.close();
}

size_t Archiver::ReadRequiredSizeForPackageData(size_t hash)
{
	auto& packageTableEntry = m_CompressedPackage.table.find(hash);

	if (packageTableEntry == m_CompressedPackage.table.end())
		return 0;

	return packageTableEntry->second.uncompressedSize;
}

void Archiver::ReadPackageData(size_t hash, void* pBuf, size_t bufSize)
{
	auto& packageTableEntry = m_CompressedPackage.table.find(hash);

	if(packageTableEntry == m_CompressedPackage.table.end())
		return;

	if (bufSize < packageTableEntry->second.uncompressedSize)
		return;

	void* pCompressedStart = reinterpret_cast<void*>((size_t)m_CompressedPackage.pData + packageTableEntry->second.offset);

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
}

void Archiver::CloseCompressedPackage()
{
	assert(m_CompressedPackage.isOpen);

	m_CompressedPackage.Reset();
}

void Archiver::CreateUncompressedPackage()
{
	m_UncompressedPackageEntries.clear();
}

void Archiver::AddToUncompressedPackage(size_t hash, size_t sizeInBytes, void* pData)
{
	void* pDataCopy = MemoryManager::GetInstance().Allocate(sizeInBytes, 1, "Uncompressed Package Data");
	memcpy(pDataCopy, pData, sizeInBytes);
	m_UncompressedPackageEntries[hash] = PackageEntry(sizeInBytes, 0, pDataCopy);
}

void Archiver::RemoveFromUncompressedPackage(size_t hash)
{
	m_UncompressedPackageEntries.erase(hash);
}

void Archiver::SaveUncompressedPackage(const std::string& filename)
{
	std::unordered_map<size_t, PackageEntry> compressedPackageEntries;

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
		
		void* pCompressed = MemoryManager::GetInstance().Allocate(it.second.uncompressedSize, 1, "Archiver Compressed Buffer");
		compressionStream.next_in = reinterpret_cast<Byte*>(it.second.pData);
		compressionStream.next_out = reinterpret_cast<Byte*>(pCompressed);
		compressionStream.avail_in = it.second.uncompressedSize;
		compressionStream.avail_out = it.second.uncompressedSize;

		err = deflate(&compressionStream, Z_FINISH);

		if (err == Z_OK || err == Z_BUF_ERROR)
		{
			//Compressed Size is larger than Uncompressed Size
			err = deflateEnd(&compressionStream);
			ARCHIVER_CHECK_ERR(err, "deflateEnd");

			memcpy(pCompressed, it.second.pData, it.second.uncompressedSize);
			compressedPackageEntries[it.first] = PackageEntry(compressedDataSize, it.second.uncompressedSize, 0, pCompressed);
			compressedDataSize += it.second.uncompressedSize;
		}
		else if (err == Z_STREAM_END)
		{
			err = deflateEnd(&compressionStream);
			ARCHIVER_CHECK_ERR(err, "deflateEnd");

			compressedPackageEntries[it.first] = PackageEntry(compressedDataSize, it.second.uncompressedSize, compressionStream.total_out, pCompressed);
			compressedDataSize += compressionStream.total_out;
		}

#ifdef _DEBUG
		uncompressedDataSize += it.second.uncompressedSize;
#endif

		MemoryManager::GetInstance().Free(pCompressed);
	}

	std::ofstream file;
	file.open(filename + ".chat", std::ios::out | std::ios::trunc | std::ios::binary);

	file << compressedPackageEntries.size() << std::endl;
	file << compressedDataSize << std::endl;

	for (auto& it : compressedPackageEntries)
	{
		file << it.first << std::endl;
		file << it.second.offset << std::endl;
		file << it.second.uncompressedSize << std::endl;
		file << it.second.compressedSize << std::endl;
	}

	for (auto& it : compressedPackageEntries)
	{
		if (it.second.compressedSize > 0)
			file.write(reinterpret_cast<char*>(it.second.pData), it.second.compressedSize);
		else
			file.write(reinterpret_cast<char*>(it.second.pData), it.second.uncompressedSize);
	}

	file.close();

#ifdef _DEBUG
	std::cout << std::endl << "Compressed Package: " << std::endl;
	std::cout << "Uncompressed Size: " << uncompressedDataSize << " bytes" << std::endl;
	std::cout << "Compressed Size: " << compressedDataSize << " bytes" << std::endl;
	std::cout << "Compressed Relative Size: " << (100.0f * (float)compressedDataSize / uncompressedDataSize) << "%" << std::endl << std::endl;

	{
		std::ofstream debugFile;
		debugFile.open(filename + "_debug.txt", std::ios::out | std::ios::trunc);

		debugFile << "Table Enties: " << compressedPackageEntries.size() << std::endl;
		debugFile << "Data Size: " << compressedDataSize << std::endl;

		for (auto& it : compressedPackageEntries)
		{
			debugFile << "Hash: " << it.first << std::endl;
			debugFile << "Offset: " << it.second.offset << std::endl;
			debugFile << "UncompressedSize: " << it.second.uncompressedSize << std::endl;
			debugFile << "CompressedSize: " << it.second.compressedSize << std::endl;
		}

		debugFile << "Data: ";
		for (auto& it : compressedPackageEntries)
		{
			debugFile.write(reinterpret_cast<char*>(it.second.pData), it.second.compressedSize);
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
