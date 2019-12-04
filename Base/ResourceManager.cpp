#include "ResourceManager.h"
#include "Archiver.h"
#include "ResourceLoader.h"
#include "TaskManager.h"
#include "ResourceBundle.h"
#include <mutex>

ResourceManager::ResourceManager()
	: m_IsCleanup(false)
{

}

ResourceManager::~ResourceManager()
{
	{
		std::scoped_lock<SpinLock> lock(m_LockLoaded);
		m_IsCleanup = true;
		for (auto resource : m_LoadedResources)
		{
			resource.second->InternalRelease();
		}
		m_LoadedResources.clear();
	}
}

void ResourceManager::LoadResource(ResourceLoader& resourceLoader, Archiver& archiver, size_t guid)
{
	size_t size = archiver.ReadRequiredSizeForPackageData(guid);
	void* data = malloc(size);
	size_t typeHash;
	archiver.ReadPackageData(guid, typeHash, data, size);

	IResource* resource = resourceLoader.LoadResourceFromMemory(data, size, typeHash);
	free(data);

	{
		std::scoped_lock<SpinLock> lock(m_LockLoaded);
		m_LoadedResources.insert({ guid, resource });
	}

	{
		std::scoped_lock<SpinLock> lock(m_LockInitiate);
		m_ResourcesToInitiate.push_back(guid);
	}
}

IResource* ResourceManager::GetResource(size_t guid)
{
	std::unordered_map<size_t, IResource*>::const_iterator iterator = m_LoadedResources.find(guid);
	if (iterator == m_LoadedResources.end())
	{
		std::cout << "Resource not found!" << std::endl;
		return nullptr;
	}

	return iterator->second;
}

Ref<ResourceBundle> ResourceManager::LoadResources(std::initializer_list<size_t> guids)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();

	archiver.OpenCompressedPackage(PACKAGE_PATH, Archiver::LOAD_AND_PREPARE);

	size_t* guidArray = new size_t[guids.size()];
	int index = 0;
	for (size_t guid : guids)
	{
		std::unordered_map<size_t, IResource*>::const_iterator iterator = m_LoadedResources.find(guid);
		if (iterator == m_LoadedResources.end())
			LoadResource(resourceLoader, archiver, guid);

		guidArray[index++] = guid;
	}

	return Ref<ResourceBundle>(new ResourceBundle(guidArray, guids.size()));
}

Ref<ResourceBundle> ResourceManager::LoadResources(std::initializer_list<char*> files)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();

	archiver.OpenCompressedPackage(PACKAGE_PATH, Archiver::LOAD_AND_PREPARE);

	size_t* guidArray = new size_t[files.size()];
	int index = 0;
	for (const char* file : files)
	{
		size_t guid = HashString(file);
		std::unordered_map<size_t, IResource*>::const_iterator iterator = m_LoadedResources.find(guid);
		if (iterator == m_LoadedResources.end())
			LoadResource(resourceLoader, archiver, guid);

		guidArray[index++] = guid;
	}

	return Ref<ResourceBundle>(new ResourceBundle(guidArray, files.size()));
}

void ResourceManager::LoadResourcesInBackground(std::vector<char*> files, const std::function<void(const Ref<ResourceBundle>&)>& callback)
{
	TaskManager& taskManager = TaskManager::Get();
	taskManager.Execute(std::bind(&ResourceManager::BackgroundLoading, this, std::move(files), callback));
}

void ResourceManager::BackgroundLoading(std::vector<char*> files, const std::function<void(const Ref<ResourceBundle>&)>& callback)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();
	std::vector<size_t> resourcesToLoad;
	size_t* guidArray = new size_t[files.size()];
	int index = 0;

	archiver.OpenCompressedPackage(PACKAGE_PATH, Archiver::LOAD_AND_PREPARE);

	//Find resources to load
	for (const char* file : files)
	{
		size_t guid = HashString(file);
		if (!IsResourceLoaded(guid))
		{
			std::scoped_lock<SpinLock> lock(m_LockLoading);
			if (!IsResourceBeingLoaded(guid))
			{
				m_ResourcesToBeLoaded.push_back(guid);
				resourcesToLoad.push_back(guid);
			}
		}
		guidArray[index++] = guid;
	}

	//Load resources
	for (size_t guid : resourcesToLoad)
	{
		LoadResource(resourceLoader, archiver, guid);
		std::scoped_lock<SpinLock> lock(m_LockLoading);
		m_ResourcesToBeLoaded.erase(std::find(m_ResourcesToBeLoaded.begin(), m_ResourcesToBeLoaded.end(), guid));
	}

	//Wait to make sure all resources are loaded, in case the resource is loaded from another thread
	for (int i = 0; i < files.size(); i++)
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
				break;
			}
		}
	}
}

void ResourceManager::Update()
{
	if (m_ResourcesToInitiate.size() > 0)
	{
		std::scoped_lock<SpinLock> lock(m_LockInitiate);
		for (size_t guid : m_ResourcesToInitiate)
		{
			IResource* resource = GetResource(guid);
			resource->InternalInit(guid);
		}
		m_ResourcesToInitiate.clear();
	}
}

bool ResourceManager::IsResourceLoaded(size_t guid)
{
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

void ResourceManager::CreateResourcePackage(std::initializer_list<char*> files)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();

	archiver.CreateUncompressedPackage();

	void* data = malloc(4096*4096);

	for (const char* file : files)
	{
		std::string filepath = std::string(file);
		std::size_t index = filepath.find_last_of(".");
		if (index == std::string::npos)
		{
			std::cout << "Error! Tried to package a file without a type [" << file << "]" << std::endl;
			continue;
		}
		size_t typeHash = HashString(filepath.substr(index).c_str());
		size_t bytesWritten = resourceLoader.WriteResourceToBuffer(filepath, data);
		archiver.AddToUncompressedPackage(HashString(file), typeHash, bytesWritten, data);
	}
	free(data);

	archiver.SaveUncompressedPackage(PACKAGE_PATH);
	archiver.CloseUncompressedPackage();

	std::cout << "ResourcePackage [" << PACKAGE_PATH << "] Created" << std::endl;
}

ResourceManager& ResourceManager::Get()
{
	static ResourceManager instance;
	return instance;
}