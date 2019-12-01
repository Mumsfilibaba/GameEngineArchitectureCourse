#pragma once
#include "Helpers.h"
#include "Mesh.h"

class MeshLoader
{
public:
	static Mesh* LoadMesh(const std::string& filepath);
private:
	static Mesh* LoadOBJ(const std::string& filepath);
};