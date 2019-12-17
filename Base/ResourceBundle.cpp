#include "ResourceBundle.h"
#include "ResourceManager.h"

ResourceBundle::ResourceBundle(size_t* guids, size_t nrOfGuids) :
	m_Guids(guids),
	m_NrOfGuids(nrOfGuids)
{

}

ResourceBundle::~ResourceBundle()
{
	if (m_Guids)
	{
		mm_free(m_Guids);
	}
}

Ref<Texture> ResourceBundle::GetTexture(size_t guid)
{
	return Ref<Texture>((Texture*)ResourceManager::Get().GetResource(guid));
}

Ref<Texture> ResourceBundle::GetTexture(const std::string& file)
{
	return GetTexture(HashString(file.c_str()));
}

Ref<Mesh> ResourceBundle::GetMesh(size_t guid)
{
    return Ref<Mesh>((Mesh*)ResourceManager::Get().GetResource(guid));
}

Ref<Mesh> ResourceBundle::GetMesh(const std::string& file)
{
    return GetMesh(HashString(file.c_str()));
}

void ResourceBundle::Unload()
{
	ResourceManager& resourceManager = ResourceManager::Get();
	for (int i = 0; i < m_NrOfGuids; i++)
	{
		resourceManager.UnloadResource(m_Guids[i]);
	}
}