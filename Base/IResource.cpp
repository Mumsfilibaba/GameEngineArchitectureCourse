#include "IResource.h"
#include "ResourceManager.h"

IResource::~IResource()
{
	ResourceManager::Get().UnloadResource(this);
}

size_t IResource::GetGUID() const
{
	return m_Guid;
}

size_t IResource::GetSize() const
{
	return m_Size;
}
