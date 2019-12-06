#pragma once
#include "IRefCountable.h"

class RefBase
{
public:
	inline RefBase(IRefCountable* refCountable)
	{
		m_RefCountable = refCountable;

		if (m_RefCountable)
			m_RefCountable->AddRef();
	};

	virtual inline ~RefBase()
	{
		if (m_RefCountable)
			m_RefCountable->RemoveRef();
	};

	RefBase& operator=(const RefBase& other)
	{
		if (&other == this)
			return *this;

		if (m_RefCountable)
			m_RefCountable->RemoveRef();

		m_RefCountable = other.m_RefCountable;
		if (m_RefCountable)
			m_RefCountable->AddRef();

		return *this;
	}

	operator bool()
	{
		return m_RefCountable;
	}

protected:
	IRefCountable* m_RefCountable;
};

template <typename T>
class Ref : public RefBase
{
public:
	inline Ref() : Ref(nullptr)
	{
		
	};

	inline Ref(IRefCountable* refCountable) : RefBase(refCountable)
	{

	};

	inline ~Ref()
	{
		
	};

	inline T* Get() const
	{
		return (T*)m_RefCountable;
	}
};