#include "LoaderCOLLADA.h"

IResource* LoaderCOLLADA::LoadFromDisk(const std::string& file)
{
	return nullptr;
}


IResource* LoaderCOLLADA::LoadFromMemory(void* data, size_t size)
{
	return nullptr;
}


size_t LoaderCOLLADA::WriteToBuffer(const std::string& file, void* buffer)
{
	return size_t();
}


std::vector<GameMesh> LoaderCOLLADA::ReadFromDisk(const std::string& filepath)
{
	//Read in the full textfile into a buffer
	const char* buffer = nullptr;
	uint32_t bytes = ReadTextfile(filepath, &buffer);
	if (bytes == 0)
	{
		ThreadSafePrintf("Failed to load '%s'\n", filepath.c_str());
		return std::vector<GameMesh>();
	}
	


	return std::vector<GameMesh>();
}


