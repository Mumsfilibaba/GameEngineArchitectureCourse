#include "ResourceManager.h"
#include "Archiver.h"
#include "ResourceLoader.h"
#include "TaskManager.h"
#include "ResourceBundle.h"

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	for (auto resource : m_LoadedResources)
	{
		resource.second->Release();
		delete resource.second;
	}

	for (auto bundle : m_ResourceBundles)
	{
		delete bundle;
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
	m_LoadedResources.insert({ guid, resource });

	m_LockInitiate.lock();
	m_ResourcesToInitiate.push_back(guid);
	m_LockInitiate.unlock();
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

ResourceBundle* ResourceManager::LoadResources(std::initializer_list<size_t> guids)
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

	ResourceBundle* resourceBundle = new ResourceBundle(guidArray, guids.size());
	m_ResourceBundles.push_back(resourceBundle);
	return resourceBundle;
}

ResourceBundle* ResourceManager::LoadResources(std::initializer_list<char*> files)
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

	ResourceBundle* resourceBundle = new ResourceBundle(guidArray, files.size());
	m_ResourceBundles.push_back(resourceBundle);
	return resourceBundle;
}

void ResourceManager::LoadResourcesInBackground(std::initializer_list<char*> files, const std::function<void(ResourceBundle*)>& callback)
{
	TaskManager& taskManager = TaskManager::Get();
	taskManager.Execute(std::bind(&ResourceManager::BackgroundLoading, this, files, callback));
}

void ResourceManager::BackgroundLoading(std::initializer_list<char*> files, const std::function<void(ResourceBundle*)>& callback)
{
	Archiver& archiver = Archiver::GetInstance();
	ResourceLoader& resourceLoader = ResourceLoader::Get();
	std::vector<size_t> resourcesToLoad;
	size_t* guidArray = new size_t[files.size()];
	int index = 0;

	//Find resources to load
	for (const char* file : files)
	{
		size_t guid = HashString(file);
		if (!IsResourceLoaded(guid))
		{
			m_LockLoading.lock();
			if (!IsResourceBeingLoaded(guid))
			{
				m_ResourcesToBeLoaded.push_back(guid);
				resourcesToLoad.push_back(guid);
			}
			m_LockLoading.unlock();
		}
		guidArray[index++] = guid;
	}

	//Load resources
	for (size_t guid : resourcesToLoad)
	{
		LoadResource(resourceLoader, archiver, guid);
		m_LockLoading.lock();
		m_ResourcesToBeLoaded.erase(std::find(m_ResourcesToBeLoaded.begin(), m_ResourcesToBeLoaded.end(), guid));
		m_LockLoading.unlock();
	}

	//Wait to make sure all resources are loaded, incase the resource is loaded from another thread
	for (int i = 0; i < files.size(); i++)
	{
		size_t guid = guidArray[i];
		while (!IsResourceLoaded(guid)){}
	}

	ResourceBundle* resourceBundle = new ResourceBundle(guidArray, files.size());
	m_ResourceBundles.push_back(resourceBundle);
	callback(resourceBundle);
}

void ResourceManager::Update()
{
	if (m_ResourcesToInitiate.size() > 0)
	{
		m_LockInitiate.lock();
		for (size_t guid : m_ResourcesToInitiate)
		{
			IResource* resource = GetResource(guid);
			resource->Init();
		}
		m_ResourcesToInitiate.clear();
		m_LockInitiate.unlock();
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
}

ResourceManager& ResourceManager::Get()
{
	static ResourceManager instance;
	return instance;
}