#include "ResourceBundle.h"

ResourceBundle::ResourceBundle(size_t* guids, size_t nrOfGuids) :
	m_Guids(guids),
	m_NrOfGuids(nrOfGuids)
{

}

Texture* getTexture(const std::string& file)
{
	return NULL;
}

Sound* getSound(const std::string& file)
{
	return NULL;
}