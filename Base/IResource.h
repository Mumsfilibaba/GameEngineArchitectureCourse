#pragma once

// I need ref counting

class IResource
{
public:
    virtual ~IResource() = default;
	virtual void Release() = 0;
};
