#pragma once

#include "Helpers.h"
// I need ref counting

class IResource
{
	friend class ResourceManager;

public:
    virtual ~IResource() = default;

protected:
	virtual void Init() = 0;
	virtual void Release() = 0;

private:

	size_t m_Guid;

	inline void InternalInit(size_t guid)
	{
		m_Guid = guid;
		Init();
#ifdef _DEBUG
		ThreadSafePrintf("Resource Initiated [%lu]\n", m_Guid);
#endif
	};

	inline void InternalRelease()
	{
		Release();
#ifdef _DEBUG
		ThreadSafePrintf("Resource Released [%lu]\n", m_Guid);
#endif
	};
};
