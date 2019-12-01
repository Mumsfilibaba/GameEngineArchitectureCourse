#include "MeshLoader.h"

Mesh* MeshLoader::LoadMesh(const char* pFilename)
{
	assert(pFilename != nullptr);

	//Check if the string we passed in contains the .obj ending, otherwise unsupported
	if (strstr(pFilename, ".obj") != nullptr)
		return LoadOBJ(pFilename);

	//No supported format
	ThreadSafePrintf("%s is not in a supported format\n", pFilename);
	return nullptr;
}


Mesh* MeshLoader::LoadOBJ(const char* pFilepath)
{
	return nullptr;
}
