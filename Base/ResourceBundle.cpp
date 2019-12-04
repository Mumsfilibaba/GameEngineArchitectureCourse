#include "ResourceBundle.h"
#include "ResourceManager.h"

ResourceBundle::ResourceBundle(size_t* guids, size_t nrOfGuids) :
	m_Guids(guids),
	m_NrOfGuids(nrOfGuids)
{

}

ResourceBundle::~ResourceBundle()
{
	if(m_Guids)
		delete[] m_Guids;
}

Ref<Texture> ResourceBundle::GetTexture(size_t guid)
{
	return Ref<Texture>((Texture*)ResourceManager::Get().GetResource(guid));
}

Ref<Texture> ResourceBundle::GetTexture(const std::string& file)
{
	return GetTexture(HashString(file.c_str()));
}

Ref<Sound> ResourceBundle::GetSound(size_t guid)
{
	return Ref<Sound>((Sound*)ResourceManager::Get().GetResource(guid));
}

Ref<Sound> ResourceBundle::GetSound(const std::string& file)
{
	return GetSound(HashString(file.c_str()));
}

Ref<Mesh> ResourceBundle::GetMesh(size_t guid)
{
    return Ref<Mesh>((Mesh*)ResourceManager::Get().GetResource(guid));
}

Ref<Mesh> ResourceBundle::GetMesh(const std::string& file)
{
    return GetMesh(HashString(file.c_str()));
}


