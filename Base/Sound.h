#pragma once
#include "IResource.h"

class Sound : public IResource
{
public:
	virtual void Init() override {};
	virtual void Release() override {};
};