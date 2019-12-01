#pragma once

// I need ref counting

class IResource
{
public:
	virtual void Release() = 0;
};