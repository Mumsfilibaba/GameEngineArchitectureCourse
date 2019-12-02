#pragma once
#include <vector>
#include <string>
#include "Texture.h"
#include "Sound.h"
#include "Mesh.h"

class ResourceBundle
{
public:
	ResourceBundle(size_t* guids, size_t nrOfGuids);

	Texture* GetTexture(size_t guid);
	Texture* GetTexture(const std::string& file);

	Sound* GetSound(size_t guid);
	Sound* GetSound(const std::string& file);

    Mesh* GetMesh(size_t guid);
    Mesh* GetMesh(const std::string& file);
private:
	size_t* m_Guids;
	size_t m_NrOfGuids;
};
