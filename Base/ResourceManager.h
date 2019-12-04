#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Helpers.h"
#include "IResource.h"
#include <algorithm>
#include <functional>
#include "SpinLock.h"


#define PACKAGE_PATH "package"

class ResourceLoader;
class Archiver;
class ResourceBundle;

class ResourceManager
{
	friend class ResourceBundle;
	friend class IResource;
	friend class Game;

public:
	~ResourceManager();

	ResourceBundle* LoadResources(std::initializer_list<size_t> guids);
	ResourceBundle* LoadResources(std::initializer_list<char*> files);

	void LoadResourcesInBackground(std::vector<char*> files, const std::function<void(ResourceBundle*)>& callback);

	bool IsResourceLoaded(size_t guid);
	bool IsResourceLoaded(const std::string& path);

	bool IsResourceBeingLoaded(size_t guid);
	bool IsResourceBeingLoaded(const std::string& path);

	void CreateResourcePackage(std::initializer_list<char*> files);

	static ResourceManager& Get();

private:
	ResourceManager();

	void LoadResource(ResourceLoader& resourceLoader, Archiver& archiver, size_t guid);
	IResource* GetResource(size_t guid);
	ResourceBundle* CreateResourceBundle(size_t* guids, size_t nrOfGuids);
	void BackgroundLoading(std::vector<char*> files, const std::function<void(ResourceBundle*)>& callback);
	void UnloadResource(IResource* resource);
	void Update();

	std::vector<ResourceBundle*> m_ResourceBundles;
	std::unordered_map<size_t, IResource*> m_LoadedResources;
	std::vector<size_t> m_ResourcesToBeLoaded;
	std::vector<size_t> m_ResourcesToInitiate;
	SpinLock m_LockLoading;
	SpinLock m_LockLoaded;
	SpinLock m_LockInitiate;
	SpinLock m_LockResourceBundles;
	bool m_IsCleanup;
};