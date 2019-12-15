#include "ResourceLoader.h"

#include <iostream>

ResourceLoader::ResourceLoader()
{

}

ResourceLoader::~ResourceLoader()
{
	for (auto loader : m_LoaderMap)
	{
		delete loader.second;
	}
}

bool ResourceLoader::RegisterLoader(const std::string& fileType, ILoader* loader)
{
	if (HasLoader(fileType))
	{
		ThreadSafePrintf("Error! A loader of the filetype [%s] is already registered!\n", fileType.c_str());
		return false;
	}

	size_t hash = HashString(fileType.c_str());
	ThreadSafePrintf("Registered [%s] Loader\n", fileType.c_str());
	m_LoaderMap.insert({ hash, loader });
	return true;
}

bool ResourceLoader::HasLoader(size_t fileTypeHash)
{
	return m_LoaderMap.find(fileTypeHash) != m_LoaderMap.end();
}

bool ResourceLoader::HasLoader(const std::string& fileType)
{
	return HasLoader(HashString(fileType.c_str()));
}

bool ResourceLoader::HasLoaderForFile(const std::string& file)
{
	std::string fileType = GetFileType(file);
	if (fileType == "")
		return false;

	return HasLoader(fileType);
}

IResource* ResourceLoader::LoadResourceFromDisk(const std::string& file)
{
	std::string fileType = GetFileType(file);
	if (fileType == "")
		return nullptr;

	ILoader* loader = GetLoader(HashString(fileType.c_str()));
	if (!loader)
	{
		ThreadSafePrintf("No loader exist for the filetype [%s]!\n", fileType.c_str());
		return nullptr;
	}

	return loader->LoadFromDisk(file);
}

IResource* ResourceLoader::LoadResourceFromMemory(void* data, size_t size, size_t typeHash, const std::string& file)
{
	ILoader* loader = GetLoader(typeHash);
	if (!loader)
	{
		ThreadSafePrintf("No loader exist for the filetype [%s]!\n", GetFileType(file).c_str());
		return nullptr;
	}

	return loader->LoadFromMemory(data, size);
}

size_t ResourceLoader::WriteResourceToBuffer(const std::string& file, void* buffer)
{
	std::string fileType = GetFileType(file);
	if (fileType == "")
		return ULLONG_MAX;

	ILoader* loader = GetLoader(HashString(fileType.c_str()));
	if (!loader)
	{
		ThreadSafePrintf("No loader exist for the filetype [%s]!\n", fileType.c_str());
		return ULLONG_MAX;
	}

	return loader->WriteToBuffer(file, buffer);
}

ILoader* ResourceLoader::GetLoader(size_t hash)
{
	std::unordered_map<size_t, ILoader*>::const_iterator iterator = m_LoaderMap.find(hash);
	if (iterator == m_LoaderMap.end())
		return nullptr;
	return iterator->second;
}

std::string ResourceLoader::GetFileType(const std::string& file)
{
	std::size_t index = file.find_last_of(".");
	if (index == std::string::npos)
	{
		ThreadSafePrintf("Error! Tried to load a file without a type [%s]!\n", file.c_str());
		return "";
	}
	return file.substr(index);
}

ResourceLoader& ResourceLoader::Get()
{
	static ResourceLoader instance;
	return instance;
}
