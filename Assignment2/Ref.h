#pragma once

#include "IRefCountable.h"
#include <type_traits>


class RefBase
{
public:
	inline RefBase(IRefCountable* refCountable)
	{
		m_RefCountable = refCountable;
		m_RefCountable->m_Refrences++;
	};

	virtual inline ~RefBase()
	{
		if (--m_RefCountable->m_Refrences)
			delete m_RefCountable;
	};

private:
	IRefCountable* m_RefCountable;
};

template <typename T>
class Ref : public RefBase
{
public:
	inline Ref(IRefCountable* refCountable) : RefBase(refCountable)
	{
		
	};

	inline ~Ref()
	{
		
	};
};