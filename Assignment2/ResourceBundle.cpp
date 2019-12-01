#include "ResourceBundle.h"
#include "ResourceManager.h"

ResourceBundle::ResourceBundle(size_t* guids, size_t nrOfGuids) :
	m_Guids(guids),
	m_NrOfGuids(nrOfGuids)
{

}

Texture* ResourceBundle::GetTexture(size_t guid)
{
	return (Texture*)ResourceManager::Get().GetResource(guid);
}

Texture* ResourceBundle::GetTexture(const std::string& file)
{
	return GetTexture(HashString(file.c_str()));
}

Sound* ResourceBundle::GetSound(size_t guid)
{
	return (Sound*)ResourceManager::Get().GetResource(guid);
}

Sound* ResourceBundle::GetSound(const std::string& file)
{
	return GetSound(HashString(file.c_str()));
}