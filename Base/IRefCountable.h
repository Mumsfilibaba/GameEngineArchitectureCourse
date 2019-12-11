#pragma once
#include <iostream>

class IRefCountable
{
	friend class RefBase;

public:
	virtual ~IRefCountable() = default;

	inline int GetRefCount() const
	{
		return m_Refrences;
	}

	inline void AddRef()
	{
		m_Refrences++;
	}

	inline void RemoveRef()
	{
		m_Refrences--;
	}

protected:
	inline IRefCountable() : m_Refrences(0) {};

	inline virtual void InternalRelease()
	{
		delete this;
	};

private:
	int m_Refrences;
};