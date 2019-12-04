#pragma once
#include <vector>
#include <string>
#include "Texture.h"
#include "Sound.h"
#include "Mesh.h"
#include "Ref.h"
#include "IRefCountable.h"

class ResourceBundle
{
public:
	ResourceBundle(size_t* guids, size_t nrOfGuids);
	~ResourceBundle();

	Ref<Texture> GetTexture(size_t guid);
	Ref<Texture> GetTexture(const std::string& file);

	Ref<Sound> GetSound(size_t guid);
	Ref<Sound> GetSound(const std::string& file);

	Ref<Mesh> GetMesh(size_t guid);
	Ref<Mesh> GetMesh(const std::string& file);

private:
	size_t* m_Guids;
	size_t m_NrOfGuids;
};
