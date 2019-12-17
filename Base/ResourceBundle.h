#pragma once
#include <vector>
#include <string>
#include "Texture.h"
#include "Mesh.h"
#include "Ref.h"
#include "IRefCountable.h"
#include "PoolAllocator.h"

class ResourceBundle : public IRefCountable
{
public:
	ResourceBundle(size_t* guids, size_t nrOfGuids);
	~ResourceBundle();

	Ref<Texture> GetTexture(size_t guid);
	Ref<Texture> GetTexture(const std::string& file);

	Ref<Mesh> GetMesh(size_t guid);
	Ref<Mesh> GetMesh(const std::string& file);

	void Unload();

	inline void* operator new(size_t size, const char* tag)
	{
#ifdef SHOW_ALLOCATIONS_DEBUG
		return PoolAllocator<ResourceBundle>::Get().AllocateBlock(tag);
#else
		return PoolAllocator<ResourceBundle>::Get().AllocateBlock(tag);
#endif
	}

	inline void operator delete(void* ptr)
	{
		PoolAllocator<ResourceBundle>::Get().FreeBlock(ptr);
	}
private:
	size_t* m_Guids;
	size_t m_NrOfGuids;
};
