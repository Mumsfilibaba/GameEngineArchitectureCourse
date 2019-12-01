#pragma once

#include <string>

#include "IResource.h"

class ILoader
{
public:
	virtual IResource* LoadFromDisk(const std::string& file) = 0;
	virtual IResource* LoadFromMemory(void* data, size_t size) = 0;
	virtual size_t WriteToBuffer(const std::string& file, void* buffer) = 0;
};