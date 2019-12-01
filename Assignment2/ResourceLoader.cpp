#include "ResourceLoader.h"

#include <iostream>

ResourceLoader::ResourceLoader()
{

}

void ResourceLoader::registerLoader(const std::string& fileType, ILoader* loader)
{
	size_t hash = HashString(fileType.c_str());
	if (m_LoaderMap.find(hash) != m_LoaderMap.end())
	{
		std::cout << "Error! A loader of the filetype hash [" << hash << "] is already registered!!!!" << std::endl;
		return;
	}
	m_LoaderMap.insert({ hash, loader });
}

IResource* ResourceLoader::loadResourceFromDisk(const std::string& file)
{
	std::size_t index = file.find_last_of(".");
	if (index == std::string::npos)
	{
		std::cout << "Error! Tried to load a file without a type [" << file << "]" << std::endl;
		return nullptr;
	}

	ILoader* loader = getLoader(HashString(file.c_str()));
	if (!loader)
		return nullptr;

	return loader->loadFromDisk(file);
}

IResource* ResourceLoader::loadResourceFromMemory(void* data, size_t size, size_t typeHash)
{
	return nullptr;
}

ILoader* ResourceLoader::getLoader(size_t hash)
{
	std::unordered_map<size_t, ILoader*>::const_iterator iterator = m_LoaderMap.find(hash);
	if (iterator == m_LoaderMap.end())
	{
		std::cout << "Error! The exist no loader for the filetype hash [" << hash << "]" << std::endl;
		return nullptr;
	}

	return iterator->second;
}

void ResourceLoader::writeResourceToBuffer(IResource* resource, void* buffer, size_t bytesWritten)
{

}

ResourceLoader& ResourceLoader::get()
{
	static ResourceLoader instance;
	return instance;
}