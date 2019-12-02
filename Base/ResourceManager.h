#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Helpers.h"
#include "IResource.h"
#include "ResourceBundle.h"

#define PACKAGE_PATH "package"

class ResourceManager
{
	friend class ResourceBundle;

public:
	~ResourceManager();

	ResourceBundle* LoadResources(std::initializer_list<size_t> guids);
	ResourceBundle* LoadResources(std::initializer_list<char*> files);

	void LoadResourcesInBackground(std::initializer_list<char*> files);

	void Callback();

	bool IsResourceLoaded(size_t guid);
	bool IsResourceLoaded(const std::string& path);

	void CreateResourcePackage(std::initializer_list<char*> files);

	static ResourceManager& Get();

private:
	ResourceManager();

	IResource* GetResource(size_t guid);

	std::vector<ResourceBundle*> m_ResourceBundles;
	std::unordered_map<size_t, IResource*> m_ResourceMap;
};