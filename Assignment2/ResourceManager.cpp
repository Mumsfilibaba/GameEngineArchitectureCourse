#include "ResourceManager.h"
#include "Archiver.h"
#include "ResourceLoader.h"

ResourceManager::ResourceManager()
{

}

ResourceBundle* ResourceManager::loadResources(std::initializer_list<size_t> guids)
{
	Archiver& archiver = Archiver::GetInstance();
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
			IResource* resource = ResourceLoader::get().loadResourceFromMemory(data, size);
			m_ResourceMap.insert({ guid, resource });
		}
		guidArray[index++] = guid;
	}

	ResourceBundle* resourceBundle = new ResourceBundle(guidArray, guids.size());
	m_ResourceBundles.push_back(resourceBundle);
	return resourceBundle;
}

ResourceBundle* ResourceManager::loadResources(std::initializer_list<char*> files)
{
	Archiver& archiver = Archiver::GetInstance();
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
			archiver.ReadPackageData(guid, data, size);

			//Create and register Resource from data
			IResource* resource = ResourceLoader::get().loadResourceFromMemory(data, size);
			m_ResourceMap.insert({ guid, resource });
		}
		guidArray[index++] = guid;
	}

	ResourceBundle* resourceBundle = new ResourceBundle(guidArray, files.size());
	m_ResourceBundles.push_back(resourceBundle);
	return resourceBundle;
}

bool ResourceManager::isResourceLoaded(size_t guid)
{
	return m_ResourceMap.find(guid) != m_ResourceMap.end();
}

bool ResourceManager::isResourceLoaded(const std::string& path)
{
	return isResourceLoaded(HashString(path.c_str()));
}

ResourceManager& ResourceManager::get()
{
	static ResourceManager instance;
	return instance;
}