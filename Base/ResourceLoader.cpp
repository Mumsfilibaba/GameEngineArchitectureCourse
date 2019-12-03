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

void ResourceLoader::RegisterLoader(const std::string& fileType, ILoader* loader)
{
	size_t hash = HashString(fileType.c_str());
	if (m_LoaderMap.find(hash) != m_LoaderMap.end())
	{
		std::cout << "Error! A loader of the filetype hash [" << hash << "] is already registered!!!!" << std::endl;
		return;
	}
	std::cout << "Loader [" << fileType << "] Registered" << std::endl;
	m_LoaderMap.insert({ hash, loader });
}

IResource* ResourceLoader::LoadResourceFromDisk(const std::string& file)
{
	std::size_t index = file.find_last_of(".");
	if (index == std::string::npos)
	{
		std::cout << "Error! Tried to load a file without a type [" << file << "]" << std::endl;
		return nullptr;
	}
	std::string fileType = file.substr(index);

	ILoader* loader = GetLoader(HashString(fileType.c_str()));
	if (!loader)
		return nullptr;

	return loader->LoadFromDisk(file);
}

IResource* ResourceLoader::LoadResourceFromMemory(void* data, size_t size, size_t typeHash)
{
	ILoader* loader = GetLoader(typeHash);
	if (!loader)
		return nullptr;

	return loader->LoadFromMemory(data, size);
}

size_t ResourceLoader::WriteResourceToBuffer(const std::string& file, void* buffer)
{
	std::size_t index = file.find_last_of(".");
	if (index == std::string::npos)
	{
		std::cout << "Error! Tried to load a file without a type [" << file << "]" << std::endl;
		return size_t(-1);
	}
	std::string fileType = file.substr(index);

	ILoader* loader = GetLoader(HashString(fileType.c_str()));
	if (!loader)
		return size_t(-1);

	return loader->WriteToBuffer(file, buffer);
}

ILoader* ResourceLoader::GetLoader(size_t hash)
{
	std::unordered_map<size_t, ILoader*>::const_iterator iterator = m_LoaderMap.find(hash);
	if (iterator == m_LoaderMap.end())
	{
		std::cout << "Error! The exist no loader for the filetype hash [" << hash << "]" << std::endl;
		return nullptr;
	}

	return iterator->second;
}

ResourceLoader& ResourceLoader::Get()
{
	static ResourceLoader instance;
	return instance;
}