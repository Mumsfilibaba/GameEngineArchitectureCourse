#pragma once
#include "Helpers.h"
#include "Mesh.h"
#include "ILoader.h"

using GameMesh = TMesh<Vertex>;

class LoaderOBJ : public ILoader
{
public:
	virtual IResource* LoadFromDisk(const std::string& file) override;
	virtual IResource* LoadFromMemory(void* data, size_t size) override;
	virtual size_t WriteToBuffer(const std::string& file, void* buffer) override;
public:
	static std::vector<GameMesh> ReadFromDisk(const std::string& filepath);
};
