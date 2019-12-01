#pragma once

#include <string>

#include "IResource.h"

class ILoader
{
public:
	virtual IResource* loadFromDisk(const std::string& file) = 0;
	virtual IResource* loadFromMemory(void* data, size_t size) = 0;
	virtual void writeToBuffer(IResource* resource, void* buffer, size_t bytesWritten) = 0;
};