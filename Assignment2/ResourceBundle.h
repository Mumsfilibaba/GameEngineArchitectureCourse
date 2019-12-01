#pragma once
#include <vector>
#include "Texture.h"
#include "Sound.h"

class ResourceBundle
{
public:
	ResourceBundle(size_t* guids, size_t nrOfGuids);

	Texture* getTexture(const std::string& file);
	Sound* getSound(const std::string& file);

private:
	size_t* m_Guids;
	size_t m_NrOfGuids;
};