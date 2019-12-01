#pragma once

#include <string>
#include <unordered_map>

#include "ILoader.h"
#include "StringHash.h"

class ResourceLoader
{
	friend class ResourceManager;
public:

	void RegisterLoader(const std::string& fileType, ILoader* loader);
	IResource* LoadResourceFromDisk(const std::string& file);

	static ResourceLoader& Get();

private:
	ResourceLoader();

	IResource* LoadResourceFromMemory(void* data, size_t size, size_t typeHash);
	size_t WriteResourceToBuffer(const std::string& file, void* buffer);
	ILoader* GetLoader(size_t hash);

	std::unordered_map<size_t, ILoader*> m_LoaderMap;
};