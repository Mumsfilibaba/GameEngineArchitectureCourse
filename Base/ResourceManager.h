#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Helpers.h"
#include "IResource.h"
#include <algorithm>
#include <functional>
#include "SpinLock.h"
#include "Ref.h"


#define PACKAGE_PATH "package"
#define RESOURCE_MANAGER_MAX_MEMORY 4096 * 4096 * 3

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

	Ref<ResourceBundle> LoadResources(std::vector<std::string> files);

	void LoadResourcesInBackground(std::vector<std::string> files, const std::function<void(const Ref<ResourceBundle>&)>& callback);
	IResource* GetResource(size_t guid);
	IResource* GetResource(const std::string& file);
	IResource* GetStrongResource(const std::string& file);

	IResource * GetStrongResource(size_t guid);

	bool IsResourceLoaded(size_t guid);
	bool IsResourceLoaded(const std::string& path);

	bool IsResourceBeingLoaded(size_t guid);
	bool IsResourceBeingLoaded(const std::string& path);

	bool UnloadResource(size_t guid);

	void CreateResourcePackage(const std::string& directory, std::vector<char*>& fileNames);

	size_t GetMaxMemory() const;
	size_t GetUsedMemory() const;
	size_t GetNrOfResourcesLoaded() const;
	size_t GetNrOfResourcesInUse() const;

	void GetResourcesInUse(std::vector<IResource*>& vector);
	void GetResourcesLoaded(std::vector<IResource*>& vector);

	static ResourceManager& Get();

private:
	ResourceManager();

	bool LoadResource(ResourceLoader& resourceLoader, Archiver& archiver, size_t guid, const std::string& file);
	void BackgroundLoading(std::vector<std::string> files, const std::function<void(const Ref<ResourceBundle>&)>& callback);
	void UnloadResource(IResource* resource);
	void UnloadUnusedResources(bool force = false);
	void Update();

	bool IsResourceBeingLoadedInternal(size_t guid);

	std::unordered_map<size_t, IResource*> m_LoadedResources;
	std::vector<size_t> m_ResourcesToBeLoaded;
	std::vector<size_t> m_ResourcesToInitiate;
	SpinLock m_LockLoading;
	SpinLock m_LockLoaded;
	SpinLock m_LockInitiate;
	bool m_IsCleanup;
	size_t m_MaxMemory;
	std::atomic_uint64_t m_UsedMemory;
};
