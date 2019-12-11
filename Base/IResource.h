#pragma once

#include "Helpers.h"
#include "IRefCountable.h"

class IResource : public IRefCountable
{
	friend class ResourceManager;

public:
	IResource() : m_Guid(0) {};
	virtual ~IResource();

	size_t GetGUID() const;
	size_t GetSize() const;
	const std::string& GetName() const;

protected:
	virtual void Init() = 0;
	virtual void Release() = 0;

private:
	inline void InternalInit()
	{
		Init();
#ifdef _DEBUG
		ThreadSafePrintf("Resource Initiated [%s]\n", m_Name.c_str());
#endif
	};

	inline virtual void InternalRelease() override
	{
		Release();
#ifdef _DEBUG
		ThreadSafePrintf("Resource Released [%s]\n", m_Name.c_str());
#endif
		IRefCountable::InternalRelease();
	};

	size_t m_Guid;
	size_t m_Size;
	std::string m_Name;
};
