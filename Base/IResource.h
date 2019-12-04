#pragma once

#include "Helpers.h"
#include "IRefCountable.h"

class IResource : public IRefCountable
{
	friend class ResourceManager;

public:
	virtual ~IResource();

protected:
	virtual void Init() = 0;
	virtual void Release() = 0;

private:
	inline void InternalInit(size_t guid)
	{
		m_Guid = guid;
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
};
