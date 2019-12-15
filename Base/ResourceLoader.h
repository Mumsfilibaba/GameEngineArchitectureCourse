#pragma once

#include <string>
#include <unordered_map>

#include "ILoader.h"
#include "Helpers.h"

class ResourceLoader
{
	friend class ResourceManager;
public:
	~ResourceLoader();

	bool RegisterLoader(const std::string& fileType, ILoader* loader);
	bool HasLoader(size_t fileTypeHash);
	bool HasLoader(const std::string& fileType);
	bool HasLoaderForFile(const std::string& file);

	IResource* LoadResourceFromDisk(const std::string& file);

	static ResourceLoader& Get();

private:
	ResourceLoader();

	IResource* LoadResourceFromMemory(void* data, size_t size, size_t typeHash, const std::string& file);
	size_t WriteResourceToBuffer(const std::string& file, void* buffer);
	ILoader* GetLoader(size_t hash);
	std::string GetFileType(const std::string& file);

	std::unordered_map<size_t, ILoader*> m_LoaderMap;
};