#pragma once

#include <string>
#include <unordered_map>

#include "ILoader.h"
#include "StringHash.h"

class ResourceLoader
{
	friend class ResourceManager;
public:

	void registerLoader(const std::string& fileType, ILoader* loader);
	IResource* loadResourceFromDisk(const std::string& file);
	void writeResourceToBuffer(IResource* resource, void* buffer, size_t bytesWritten);

	static ResourceLoader& get();

private:
	ResourceLoader();

	IResource* loadResourceFromMemory(void* data, size_t size);

	ILoader* getLoader(size_t hash);

	std::unordered_map<size_t, ILoader*> m_LoaderMap;
};