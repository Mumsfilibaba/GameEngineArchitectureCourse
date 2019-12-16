#include "ResourceManager.h"
#include "Archiver.h"
#include "ResourceLoader.h"
#include "TaskManager.h"
#include "ResourceBundle.h"
#include <mutex>

ResourceManager::ResourceManager()
	: m_IsCleanup(false),
	m_MaxMemory(RESOURCE_MANAGER_MAX_MEMORY),
	m_UsedMemory(0)
{

}

ResourceManager::~ResourceManager()
{
	ResourceManager::UnloadUnusedResources(true);
}

bool ResourceManager::LoadResource(ResourceLoader& resourceLoader, Archiver& archiver, size_t guid, const std::string& file)
{
	size_t size = archiver.ReadRequiredSizeForPackageData(guid);
	if (size == 0)
	{
		ThreadSafePrintf("Failed to read size of [%s]!\n", file.c_str());
		return false;
	}

	if (m_UsedMemory + size > m_MaxMemory)
	{
		ThreadSafePrintf("No more memory available, will try to release unused resources!\n");
		UnloadUnusedResources();
		if (m_UsedMemory + size > m_MaxMemory)
		{
			ThreadSafePrintf("Error! No more memory available for [%s]!\n", file.c_str());
			return false;
		}
	}

	m_UsedMemory += size;

	void* data = malloc(size);
	size_t typeHash;
	if (!archiver.ReadPackageData(guid, typeHash, data, size))
	{
		ThreadSafePrintf("Failed to load resource data [%s]!\n", file.c_str());
		free(data);
		return false;
	}

	IResource* resource = resourceLoader.LoadResourceFromMemory(data, size, typeHash, file);
	if (!resource)
	{
		ThreadSafePrintf("Failed to create resource [%s]!\n", file.c_str());
		free(data);
		return false;
	}

	resource->m_Guid = guid;
	resource->m_Size = size;
	resource->m_Name = file;
	free(data);

	{
		std::scoped_lock<SpinLock> lock(m_LockLoaded);
		m_LoadedResources.insert({ guid, resource });
	}

	{
		std::scoped_lock<SpinLock> lock(m_LockInitiate);
		m_ResourcesToInitiate.push_back(guid);
	}
	return true;
}

IResource* ResourceManager::GetResource(size_t guid)
{
	std::unordered_map<size_t, IResource*>::const_iterator iterator = m_LoadedResources.find(guid);
	if (iterator == m_LoadedResources.end())
	{
		ThreadSafePrintf("Resource not found! [%lu]\n", guid);
		return nullptr;
	}

	return iterator->second;
}

IResource* ResourceManager::GetResource(const std::string& file)
{
	std::unordered_map<size_t, IResource*>::const_iterator iterator = m_LoadedResources.find(HashString(file.c_str()));
	if (iterator == m_LoadedResources.end())
	{
		ThreadSafePrintf("Resource not found! [%s]\n", file.c_str());
		return nullptr;
	}
	return iterator->second;
}

Ref<ResourceBundle> ResourceManager::LoadResources(std::vector<std::string> files)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();

	archiver.OpenCompressedPackage(PACKAGE_PATH, Archiver::LOAD_AND_PREPARE);

	size_t* guidArray = new size_t[files.size()];
	int index = 0;
	for (std::string& file : files)
	{
		size_t guid = HashString(file.c_str());
		std::unordered_map<size_t, IResource*>::const_iterator iterator = m_LoadedResources.find(guid);
		if (iterator == m_LoadedResources.end())
		{
			if (!LoadResource(resourceLoader, archiver, guid, file))
			{
				delete[] guidArray;
				return Ref<ResourceBundle>();
			}
			ThreadSafePrintf("Loaded [%s]\n", file.c_str());
		}
			
		guidArray[index++] = guid;
	}

	return Ref<ResourceBundle>(new ResourceBundle(guidArray, files.size()));
}

void ResourceManager::LoadResourcesInBackground(std::vector<std::string> files, const std::function<void(const Ref<ResourceBundle>&)>& callback)
{
	TaskManager& taskManager = TaskManager::Get();
	taskManager.Execute(std::bind(&ResourceManager::BackgroundLoading, this, std::move(files), callback));
}

void ResourceManager::BackgroundLoading(std::vector<std::string> files, const std::function<void(const Ref<ResourceBundle>&)>& callback)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();
	std::vector<std::pair<size_t, std::string>> resourcesToLoad;
	size_t* guidArray = new size_t[files.size()];
	int index = 0;

	archiver.OpenCompressedPackage(PACKAGE_PATH, Archiver::LOAD_AND_PREPARE);

	//Find resources to load
	for (std::string file : files)
	{
		size_t guid = HashString(file.c_str());
		if (!IsResourceLoaded(guid))
		{
			std::scoped_lock<SpinLock> lock(m_LockLoading);
			if (!IsResourceBeingLoaded(guid))
			{
				m_ResourcesToBeLoaded.push_back(guid);
				resourcesToLoad.push_back({ guid, file });
			}
		}
		guidArray[index++] = guid;
	}

	//Load resources
	for (auto& pair1 : resourcesToLoad)
	{
		if (!LoadResource(resourceLoader, archiver, pair1.first, pair1.second))
		{
			delete[] guidArray;
			callback(Ref<ResourceBundle>());
			std::scoped_lock<SpinLock> lock(m_LockLoading);
			for (auto& pair2 : resourcesToLoad)
			{
				m_ResourcesToBeLoaded.erase(std::find(m_ResourcesToBeLoaded.begin(), m_ResourcesToBeLoaded.end(), pair2.first));
			}
			return;
		}
		ThreadSafePrintf("Loaded [%s] in background!\n", pair1.second.c_str());
		std::scoped_lock<SpinLock> lock(m_LockLoading);
		m_ResourcesToBeLoaded.erase(std::find(m_ResourcesToBeLoaded.begin(), m_ResourcesToBeLoaded.end(), pair1.first));
	}

	//Wait to make sure all resources are loaded, in case the resource is loaded from another thread
	for (size_t i = 0; i < files.size(); i++)
	{
		size_t guid = guidArray[i];
		while (!IsResourceLoaded(guid)){}
	}

	callback(Ref<ResourceBundle>(new ResourceBundle(guidArray, files.size())));
}

void ResourceManager::UnloadResource(IResource* resource)
{
	if (!m_IsCleanup)
	{
		std::scoped_lock<SpinLock> lock(m_LockLoaded);
		for (auto it = m_LoadedResources.begin(); it != m_LoadedResources.end(); ++it)
		{
			if (it->second == resource)
			{
				m_LoadedResources.erase(it);
				m_UsedMemory -= resource->m_Size;
				break;
			}
		}
	}
}

void ResourceManager::UnloadUnusedResources(bool force)
{
	std::scoped_lock<SpinLock> lock(m_LockLoaded);
	std::vector<IResource*> resourcesToUnload;
	m_IsCleanup = true;
	bool exit = false;
	while (!exit)
	{
		exit = true;
		for (auto it = m_LoadedResources.begin(); it != m_LoadedResources.end(); it++)
		{
			IResource* res = it->second;
			if (res->GetRefCount() == 0 || force)
			{
				m_LoadedResources.erase(it);
				m_UsedMemory -= res->m_Size;
				res->InternalRelease();
				exit = false;
				break;
			}
		}
	}
	m_IsCleanup = false;
}

void ResourceManager::Update()
{
	if (m_ResourcesToInitiate.size() > 0)
	{
		std::scoped_lock<SpinLock> lock(m_LockInitiate);
		for (size_t guid : m_ResourcesToInitiate)
		{
			IResource* resource = GetResource(guid);
			if(resource)
				resource->InternalInit();
		}
		m_ResourcesToInitiate.clear();
	}
}

bool ResourceManager::IsResourceLoaded(size_t guid)
{
	std::scoped_lock<SpinLock> lock(m_LockLoaded);
	return m_LoadedResources.find(guid) != m_LoadedResources.end();
}

bool ResourceManager::IsResourceLoaded(const std::string& path)
{
	return IsResourceLoaded(HashString(path.c_str()));
}

bool ResourceManager::IsResourceBeingLoaded(size_t guid)
{
	return std::find(m_ResourcesToBeLoaded.begin(), m_ResourcesToBeLoaded.end(), guid) != m_ResourcesToBeLoaded.end();
}

bool ResourceManager::IsResourceBeingLoaded(const std::string& path)
{
	return IsResourceBeingLoaded(HashString(path.c_str()));
}

bool ResourceManager::UnloadResource(size_t guid)
{
	IResource* resource = GetResource(guid);
	if (!resource)
		return true;

	if (resource->GetRefCount() == 0)
	{
		std::scoped_lock<SpinLock> lock(m_LockLoaded);
		m_IsCleanup = true;
		for (auto it = m_LoadedResources.begin(); it != m_LoadedResources.end(); ++it)
		{
			if (it->second == resource)
			{
				m_LoadedResources.erase(it);
				m_UsedMemory -= resource->m_Size;
				resource->InternalRelease();
				m_IsCleanup = false;
				return true;
			}
		}
		m_IsCleanup = false;
	}
    
	ThreadSafePrintf("Failed to unload resource becouse it is currently being used [%s]", resource->GetName().c_str());
    return false;
}

void ResourceManager::CreateResourcePackage(const std::string& directory, std::vector<char*>& fileNames)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();

	archiver.CreateUncompressedPackage();

	void* data = malloc(4096 * 4096 * 4);

	for (const char* file : fileNames)
	{
		std::string fileName = std::string(file);
		std::string filePath = directory + fileName;
		std::size_t index = fileName.find_last_of(".");
		if (index == std::string::npos)
		{
			ThreadSafePrintf("Error! Tried to package a file without a type [%s]\n", file);
			continue;
		}
		size_t typeHash = HashString(fileName.substr(index).c_str());
		size_t bytesWritten = resourceLoader.WriteResourceToBuffer(filePath, data);
		if (bytesWritten == ULLONG_MAX)
		{
			ThreadSafePrintf("Error! Failed to package the following file [%s]\n", file);
			continue;
		}
		archiver.AddToUncompressedPackage(HashString(file), typeHash, bytesWritten, data);
	}
	free(data);

	archiver.SaveUncompressedPackage(PACKAGE_PATH);
	archiver.CloseUncompressedPackage();

	ThreadSafePrintf("ResourcePackage [%s] Created\n", PACKAGE_PATH);
}

size_t ResourceManager::GetMaxMemory() const
{
	return m_MaxMemory;
}

size_t ResourceManager::GetUsedMemory() const
{
	return m_UsedMemory;
}

size_t ResourceManager::GetNrOfResourcesLoaded() const
{
	return m_LoadedResources.size();
}

size_t ResourceManager::GetNrOfResourcesInUse() const
{
	size_t resourcesInUse = 0;
	for(std::pair<size_t, IResource*> resource : m_LoadedResources)
	{
		if (resource.second->GetRefCount() > 0)
			resourcesInUse++;
	}
	return resourcesInUse;
}

void ResourceManager::GetResourcesInUse(std::vector<IResource*>& vector)
{
	std::scoped_lock<SpinLock> lock(m_LockLoaded);
	for (std::pair<size_t, IResource*> resource : m_LoadedResources)
	{
		if (resource.second->GetRefCount() > 0)
			vector.push_back(resource.second);
	}
}

void ResourceManager::GetResourcesLoaded(std::vector<IResource*>& vector) const
{
	for (std::pair<size_t, IResource*> resource : m_LoadedResources)
	{
		vector.push_back(resource.second);
	}
}

ResourceManager& ResourceManager::Get()
{
	static ResourceManager instance;
	return instance;
}
