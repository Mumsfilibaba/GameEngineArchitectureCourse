#pragma once

// I need ref counting

class IResource
{
public:
    virtual ~IResource() = default;

	virtual void Init() = 0;
	virtual void Release() = 0;
};
