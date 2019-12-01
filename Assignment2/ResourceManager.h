#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "StringHash.h"
#include "IResource.h"
#include "ResourceBundle.h"

#define PACKAGE_PATH "package"

class ResourceManager
{
public:

	ResourceBundle* loadResources(std::initializer_list<size_t> guids);
	ResourceBundle* loadResources(std::initializer_list<char*> files);

	bool isResourceLoaded(size_t guid);
	bool isResourceLoaded(const std::string& path);

	static ResourceManager& get();

private:
	ResourceManager();

	std::vector<ResourceBundle*> m_ResourceBundles;
	std::unordered_map<size_t, IResource*> m_ResourceMap;
};