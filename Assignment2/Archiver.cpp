#include "Archiver.h"

#define ARCHIVER_CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
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
	file.open(filename, std::ios::in);

	size_t tableLines;
	size_t dataSize;
	file >> tableLines;
	file >> dataSize;

	for (size_t i = 0; i < tableLines; i++)
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

	m_CompressedPackage.pData = MemoryManager::GetInstance().Allocate(dataSize, 1, "Package Data");
	file.read(reinterpret_cast<char*>(m_CompressedPackage.pData), dataSize);

	assert(file.eof());

	file.close();
}

void* Archiver::ReadPackageData(size_t hash, size_t& size)
{
	auto& packageTableEntry = m_CompressedPackage.table.find(hash);

	if(packageTableEntry == m_CompressedPackage.table.end())
		return nullptr;

	int err;
	z_stream decompressionStream;
	decompressionStream.zalloc = zalloc;
	decompressionStream.zfree = zfree;
	decompressionStream.opaque = nullptr;

	err = inflateInit(&decompressionStream);
	ARCHIVER_CHECK_ERR(err, "inflateInit");

	void* pDecompressed = MemoryManager::GetInstance().Allocate(packageTableEntry->second.uncompressedSize, 1, "Archiver Decompressed Buffer");
	decompressionStream.next_in = reinterpret_cast<Byte*>((size_t)m_CompressedPackage.pData + packageTableEntry->second.offset);
	decompressionStream.next_out = reinterpret_cast<Byte*>(pDecompressed);
	decompressionStream.avail_in = packageTableEntry->second.compressedSize;
	decompressionStream.avail_out = packageTableEntry->second.uncompressedSize;

	err = inflate(&decompressionStream, Z_FINISH);
	assert(err <= Z_OK);

	err = inflateEnd(&decompressionStream);
	ARCHIVER_CHECK_ERR(err, "inflateEnd");

	size = packageTableEntry->second.uncompressedSize;
	return pDecompressed;
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
	m_UncompressedPackageEntries[hash] = PackageEntry(sizeInBytes, 0, pData);
}

void Archiver::RemoveFromUncompressedPackage(size_t hash)
{
	m_UncompressedPackageEntries.erase(hash);
}

void Archiver::SaveUncompressedPackage(const std::string& filename)
{
	std::unordered_map<size_t, PackageEntry> compressedPackageEntries;
	size_t dataSize = 0;

	for (auto& it : m_UncompressedPackageEntries)
	{
		int err;
		z_stream compressionStream;
		compressionStream.zalloc = zalloc;
		compressionStream.zfree = zfree;
		compressionStream.opaque = nullptr;

		err = deflateInit(&compressionStream, Z_DEFAULT_COMPRESSION);
		ARCHIVER_CHECK_ERR(err, "deflateInit");
		
		void* pCompressed = MemoryManager::GetInstance().Allocate(it.second.uncompressedSize, 1, "Archiver Compressed Buffer");
		compressionStream.next_in = reinterpret_cast<Byte*>(it.second.pData);
		compressionStream.next_out = reinterpret_cast<Byte*>(pCompressed);
		compressionStream.avail_in = it.second.uncompressedSize;
		compressionStream.avail_out = it.second.uncompressedSize;

		err = deflate(&compressionStream, Z_FINISH);
		assert(err <= Z_OK);

		err = deflateEnd(&compressionStream);
		ARCHIVER_CHECK_ERR(err, "deflateEnd");

		compressedPackageEntries[it.first] = PackageEntry(dataSize, it.second.uncompressedSize, compressionStream.total_out, pCompressed);
		dataSize += compressionStream.total_out;
	}

	std::ofstream file;
	file.open(filename, std::ios::out | std::ios::trunc);

	file << compressedPackageEntries.size() * 2 << std::endl;
	file << dataSize << std::endl;

	for (auto& it : compressedPackageEntries)
	{
		file << it.first << std::endl;
		file << it.second.offset << std::endl;
		file << it.second.uncompressedSize << std::endl;
		file << it.second.compressedSize << std::endl;
	}

	for (auto& it : compressedPackageEntries)
	{
		file.write(reinterpret_cast<char*>(it.second.pData), it.second.compressedSize);
	}

	file.close();
}
