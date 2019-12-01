#include "ResourceManager.h"
#include "Archiver.h"
#include "ResourceLoader.h"

ResourceManager::ResourceManager()
{

}

IResource* ResourceManager::GetResource(size_t guid)
{
	std::unordered_map<size_t, IResource*>::const_iterator iterator = m_ResourceMap.find(guid);
	if (iterator == m_ResourceMap.end())
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
		std::unordered_map<size_t, IResource*>::const_iterator iterator = m_ResourceMap.find(guid);
		if (iterator == m_ResourceMap.end())
		{
			//Load data
			size_t size = archiver.ReadRequiredSizeForPackageData(guid);
			void* data = malloc(size);
			size_t typeHash;
			archiver.ReadPackageData(guid, typeHash, data, size);

			//Create and register Resource from data
			IResource* resource = resourceLoader.LoadResourceFromMemory(data, size, typeHash);
			m_ResourceMap.insert({ guid, resource });
		}
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
		std::unordered_map<size_t, IResource*>::const_iterator iterator = m_ResourceMap.find(guid);
		if (iterator == m_ResourceMap.end())
		{
			//Load data
			size_t size = archiver.ReadRequiredSizeForPackageData(guid);
			void* data = malloc(size);
			size_t typeHash;
			archiver.ReadPackageData(guid, typeHash, data, size);

			//Create and register Resource from data
			IResource* resource = resourceLoader.LoadResourceFromMemory(data, size, typeHash);
			m_ResourceMap.insert({ guid, resource });
		}
		guidArray[index++] = guid;
	}

	ResourceBundle* resourceBundle = new ResourceBundle(guidArray, files.size());
	m_ResourceBundles.push_back(resourceBundle);
	return resourceBundle;
}

bool ResourceManager::IsResourceLoaded(size_t guid)
{
	return m_ResourceMap.find(guid) != m_ResourceMap.end();
}

bool ResourceManager::IsResourceLoaded(const std::string& path)
{
	return IsResourceLoaded(HashString(path.c_str()));
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

		int bytesWritten = resourceLoader.WriteResourceToBuffer(filepath, data);
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