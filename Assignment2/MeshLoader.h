#pragma once
#include "Helpers.h"
#include "Mesh.h"

class MeshLoader
{
public:
	static Mesh* LoadMesh(const char* pFilepath);
private:
	static Mesh* LoadOBJ(const char* pFilepath);
};