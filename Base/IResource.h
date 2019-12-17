#pragma once
#include "Helpers.h"
#include "IRefCountable.h"
#include "MemoryManager.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4291)		//Disable: "no matching operator delete found; memory will not be freed if initialization throws an exception"-warning
#endif

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

	inline void* operator new(size_t size, const char* tag)
	{
#ifdef SHOW_ALLOCATIONS_DEBUG
		return mm_allocate(size, 1, tag);
#else
		return mm_allocate(size, 1, tag);
#endif
	}

	inline void operator delete(void* ptr)
	{
		mm_free(ptr);
	}
protected:
	virtual void Init() = 0;
	virtual void Release() = 0;

private:
	inline void InternalInit()
	{
		Init();
		ThreadSafePrintf("Resource Initiated [%s]\n", m_Name.c_str());
		m_Ready = true;
	};

	virtual inline void InternalRelease() override
	{
		Release();
		ThreadSafePrintf("Resource Released [%s]\n", m_Name.c_str());
		IRefCountable::InternalRelease();
	};

	size_t m_Guid;
	size_t m_Size;
	std::string m_Name;
	bool m_Ready;
};
