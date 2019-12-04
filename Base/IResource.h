#pragma once

#include "Helpers.h"
#include "IRefCountable.h"

class IResource : public IRefCountable
{
	friend class ResourceManager;

public:
	IResource() : m_Guid(0) {};
	virtual ~IResource();

protected:
	virtual void Init() = 0;
	virtual void Release() = 0;

private:
	inline void InternalInit()
	{
		Init();
#ifdef _DEBUG
		ThreadSafePrintf("Resource Initiated [%lu]\n", m_Guid);
#endif
	};

	inline virtual void InternalRelease() override
	{
		Release();
#ifdef _DEBUG
		ThreadSafePrintf("Resource Released [%lu]\n", m_Guid);
#endif
		IRefCountable::InternalRelease();
	};

	size_t m_Guid;
	size_t m_Size;
};
