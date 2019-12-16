#include "Archiver.h"

#define ARCHIVER_CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        assert(false); \
    } \
}

/*void* ArchiverAlloc(void*, unsigned num, unsigned size)
{
	return MemoryManager::GetInstance().Allocate((size_t)num * (size_t)size, 1, "zLib");
}

void ArchiverFree(void*, void* pAddress)
{
	MemoryManager::GetInstance().Free(pAddress);
}*/

void* ArchiverAlloc(void*, unsigned num, unsigned size)
{
	return calloc(num, size);
}

void ArchiverFree(void*, void* pAddress)
{
	free(pAddress);
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
	void* pDecompressedHeader = DecompressHeader(fileStream);
	std::string decompressedHeaderString = reinterpret_cast<char*>(pDecompressedHeader);
	std::stringstream headerStream(decompressedHeaderString, std::ios_base::in);

	size_t tableEntries;
	size_t dataSize;
	headerStream >> tableEntries;
	headerStream >> dataSize;

	for (size_t i = 0; i < tableEntries; i++)
	{
		size_t hash;
		size_t typeHash;
		size_t offset;
		size_t uncompressedSize;
		size_t compressedSize;

		headerStream >> hash;
		headerStream >> typeHash;
		headerStream >> offset;
		headerStream >> uncompressedSize;
		headerStream >> compressedSize;

		m_CompressedPackage.table[hash] = PackageEntryDescriptor(typeHash, offset, uncompressedSize, compressedSize);
	}

	MemoryManager::GetInstance().Free(pDecompressedHeader);
	return dataSize;
}

void Archiver::OpenCompressedPackage(const std::string& filename, PackageMode packageMode)
{
	std::scoped_lock<SpinLock> lock(m_OpenPackageLock);

	if (!m_CompressedPackage.isPackageOpen)
	{
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

					m_CompressedPackage.fileDataStart = fileStream.tellg();
					m_CompressedPackage.pFileStream = &fileStream;
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
}

void Archiver::CloseCompressedPackage()
{
	assert(m_CompressedPackage.isPackageOpen);
	assert(!m_CompressedPackage.pFileStream->is_open());

	m_CompressedPackage.Reset();
}

size_t Archiver::ReadRequiredSizeForPackageData(size_t hash)
{
	auto packageTableEntry = m_CompressedPackage.table.find(hash);
	if (packageTableEntry == m_CompressedPackage.table.end())
		return 0;

	return packageTableEntry->second.uncompressedSize;
}

bool Archiver::ReadPackageData(size_t hash, size_t& typeHash, void* pBuf, size_t bufSize)
{
	auto packageTableEntry = m_CompressedPackage.table.find(hash);
	if(packageTableEntry == m_CompressedPackage.table.end())
		return false;

	if (bufSize < packageTableEntry->second.uncompressedSize)
		return false;

	typeHash = packageTableEntry->second.typeHash;
	void* pCompressedStart = nullptr;

	switch (m_CompressedPackage.packageMode)
	{
		case LOAD_AND_STORE:
		{
			if (packageTableEntry->second.compressedSize > 0)
			{
				pCompressedStart = reinterpret_cast<void*>((size_t)m_CompressedPackage.pData + packageTableEntry->second.offset);
			}
			else
			{
				memcpy(pBuf, (void*)((size_t)m_CompressedPackage.pData + packageTableEntry->second.offset), packageTableEntry->second.uncompressedSize);
				return true;
			}
			break;
		}
		case LOAD_AND_PREPARE:
		{
			assert(m_CompressedPackage.pFileStream != nullptr);

			{
				std::scoped_lock<SpinLock> lock(m_FileStreamLock);

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
						return true;
					}
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
	decompressionStream.avail_in = (uInt)packageTableEntry->second.compressedSize;
	decompressionStream.avail_out = (uInt)packageTableEntry->second.uncompressedSize;

	err = inflate(&decompressionStream, Z_FINISH);
	assert(err > Z_OK);

	err = inflateEnd(&decompressionStream);
	ARCHIVER_CHECK_ERR(err, "inflateEnd");

	if (m_CompressedPackage.packageMode == LOAD_AND_PREPARE)
		MemoryManager::GetInstance().Free(pCompressedStart);

	return true;
}

void Archiver::CreateUncompressedPackage()
{
	m_UncompressedPackageEntries.clear();
}

void Archiver::AddToUncompressedPackage(size_t hash, size_t typeHash, size_t sizeInBytes, void* pData)
{
	void* pDataCopy = MemoryManager::GetInstance().Allocate(sizeInBytes, 1, "Uncompressed Package Data");
	memcpy(pDataCopy, pData, sizeInBytes);
	m_UncompressedPackageEntries[hash] = UncompressedPackageEntry(typeHash, sizeInBytes, 0, pDataCopy);
}

void Archiver::RemoveFromUncompressedPackage(size_t hash)
{
	m_UncompressedPackageEntries.erase(hash);
}

void Archiver::SaveUncompressedPackage(const std::string& filename)
{
#ifdef _DEBUG
	size_t uncompressedDataSize = 0;
#endif

	size_t compressedDataSize = 0;
	std::stringstream headerTableStream;
	std::stringstream compressedDataStream;

	for (auto& it : m_UncompressedPackageEntries)
	{
		int err;
		z_stream compressionStream;
		compressionStream.zalloc = zalloc;
		compressionStream.zfree = zfree;
		compressionStream.opaque = nullptr;

		err = deflateInit(&compressionStream, COMPRESSION_LEVEL);
		ARCHIVER_CHECK_ERR(err, "deflateInit");
		
		void* pCompressed = MemoryManager::GetInstance().Allocate(it.second.packageEntryDesc.uncompressedSize, 1, "Archiver Compressed Buffer");
		compressionStream.next_in = reinterpret_cast<Byte*>(it.second.pData);
		compressionStream.next_out = reinterpret_cast<Byte*>(pCompressed);
		compressionStream.avail_in = (uInt)it.second.packageEntryDesc.uncompressedSize;
		compressionStream.avail_out = (uInt)it.second.packageEntryDesc.uncompressedSize;

		err = deflate(&compressionStream, Z_FINISH);

		if (err == Z_OK || err == Z_BUF_ERROR)
		{
			//Compressed Size is larger than Uncompressed Size
			err = deflateEnd(&compressionStream);
			ARCHIVER_CHECK_ERR(err, "deflateEnd");

			headerTableStream << it.first << std::endl;
			headerTableStream << it.second.packageEntryDesc.typeHash << std::endl;
			headerTableStream << compressedDataSize << std::endl;
			headerTableStream << it.second.packageEntryDesc.uncompressedSize << std::endl;
			headerTableStream << 0 << std::endl;

			compressedDataStream.write(reinterpret_cast<char*>(it.second.pData), it.second.packageEntryDesc.uncompressedSize);

			compressedDataSize += it.second.packageEntryDesc.uncompressedSize;
		}
		else if (err == Z_STREAM_END)
		{
			err = deflateEnd(&compressionStream);
			ARCHIVER_CHECK_ERR(err, "deflateEnd");

			//if (it.first == HashString("Test String"))
				//std::cout << "Saved Data: " << reinterpret_cast<char*>(pCompressed) << std::endl;

			headerTableStream << it.first << std::endl;
			headerTableStream << it.second.packageEntryDesc.typeHash << std::endl;
			headerTableStream << compressedDataSize << std::endl;
			headerTableStream << it.second.packageEntryDesc.uncompressedSize << std::endl;
			headerTableStream << compressionStream.total_out << std::endl;

			compressedDataStream.write(reinterpret_cast<char*>(pCompressed), compressionStream.total_out);

			compressedDataSize += compressionStream.total_out;
		}

#ifdef _DEBUG
		uncompressedDataSize += it.second.packageEntryDesc.uncompressedSize;
#endif

		MemoryManager::GetInstance().Free(pCompressed);
	}

	std::string headerString;

	headerString += std::to_string(m_UncompressedPackageEntries.size()) + "\n";
	headerString += std::to_string(compressedDataSize) + "\n";
	headerString += headerTableStream.str();
	void* pHeader = MemoryManager::GetInstance().Allocate(headerString.length(), 1, "Archiver Package Uncompressed Header");
	memcpy(pHeader, headerString.c_str(), headerString.length());

	size_t compressedHeaderMaxSize = headerString.length() + 4; //4 bytes extra for potential zlib header.
	void* pCompressedHeader = MemoryManager::GetInstance().Allocate(compressedHeaderMaxSize, 1, "Archiver Package Compressed Header");
	size_t compressedHeaderSize = CompressHeader(pHeader, headerString.length(), pCompressedHeader, compressedHeaderMaxSize);

	std::ofstream file;
	file.open(filename + PACKAGE_FILE_EXTENSION, std::ios::out | std::ios::trunc | std::ios::binary);

	file << compressedHeaderSize << std::endl;
	file << headerString.length() << std::endl;
	file.write(reinterpret_cast<char*>(pCompressedHeader), compressedHeaderSize);
	file << compressedDataStream.str();

	file.close();

	MemoryManager::GetInstance().Free(pHeader);
	MemoryManager::GetInstance().Free(pCompressedHeader);

#ifdef _DEBUG
	std::cout << std::endl << "Compressed Package: " << std::endl;
	std::cout << "Uncompressed Size: " << (uncompressedDataSize + headerString.length()) << " bytes" << std::endl;
	std::cout << "Compressed Size: " << (compressedDataSize + compressedHeaderSize) << " bytes" << std::endl;
	std::cout << "Compressed Relative Size: " << (100.0f * (float)(compressedDataSize + compressedHeaderSize) / (uncompressedDataSize + headerString.length())) << "%" << std::endl << std::endl;
#endif
}

void Archiver::CloseUncompressedPackage()
{
	for (auto& it : m_UncompressedPackageEntries)
	{
		if (it.second.pData != nullptr)
		{
			MemoryManager::GetInstance().Free(it.second.pData);
			it.second.pData = nullptr;
		}
	}

	m_UncompressedPackageEntries.clear();
}

void* Archiver::DecompressHeader(std::ifstream& fileStream)
{
	size_t headerCompressedSize;
	size_t headerUncompressedSize;

	fileStream >> headerCompressedSize;
	fileStream >> headerUncompressedSize;

	void* pCompressedHeader = MemoryManager::GetInstance().Allocate(headerCompressedSize, 1, "Archiver Package Compressed Header");
	void* pDecompressedHeader = MemoryManager::GetInstance().Allocate(headerUncompressedSize, 1, "Archiver Package Uncompressed Header");
	fileStream.seekg(1, std::ios_base::cur);
	fileStream.read(reinterpret_cast<char*>(pCompressedHeader), headerCompressedSize);

	int err;
	z_stream decompressionStream;
	decompressionStream.zalloc = zalloc;
	decompressionStream.zfree = zfree;
	decompressionStream.opaque = nullptr;

	err = inflateInit(&decompressionStream);
	ARCHIVER_CHECK_ERR(err, "inflateInit");

	decompressionStream.next_in = reinterpret_cast<Byte*>(pCompressedHeader);
	decompressionStream.next_out = reinterpret_cast<Byte*>(pDecompressedHeader);
	decompressionStream.avail_in = (uInt)headerCompressedSize;
	decompressionStream.avail_out = (uInt)headerUncompressedSize;

	err = inflate(&decompressionStream, Z_FINISH);
	assert(err > Z_OK);

	err = inflateEnd(&decompressionStream);
	ARCHIVER_CHECK_ERR(err, "inflateEnd");

	MemoryManager::GetInstance().Free(pCompressedHeader);
	return pDecompressedHeader;
}

size_t Archiver::CompressHeader(void* pHeader, size_t headerSize, void* pBuf, size_t bufSize)
{
	int err;
	z_stream compressionStream;
	compressionStream.zalloc = zalloc;
	compressionStream.zfree = zfree;
	compressionStream.opaque = nullptr;

	err = deflateInit(&compressionStream, COMPRESSION_LEVEL);
	ARCHIVER_CHECK_ERR(err, "deflateInit");

	compressionStream.next_in = reinterpret_cast<Byte*>(pHeader);
	compressionStream.next_out = reinterpret_cast<Byte*>(pBuf);
	compressionStream.avail_in = (uInt)headerSize;
	compressionStream.avail_out = (uInt)bufSize;

	err = deflate(&compressionStream, Z_FINISH);

	err = deflateEnd(&compressionStream);
	ARCHIVER_CHECK_ERR(err, "deflateEnd");

	return compressionStream.total_out;
}
