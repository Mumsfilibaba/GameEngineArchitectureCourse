#include "IResource.h"
#include "ResourceManager.h"

IResource::~IResource()
{
	ResourceManager::Get().UnloadResource(this);
}
