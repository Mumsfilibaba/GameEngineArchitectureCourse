#pragma once

#include "Helpers.h"
#include "IRefCountable.h"

class IResource : public IRefCountable
{
	friend class ResourceManager;

public:
	IResource() : m_Guid(0), m_Size(0), m_Ready(false) {};
	virtual ~IResource();

	size_t GetGUID() const;
	size_t GetSize() const;
	const std::string& GetName() const;

	bool IsReady() const;

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
		m_Ready = true;
	};

	virtual inline void InternalRelease() override
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
	bool m_Ready;
};
