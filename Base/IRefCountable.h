#pragma once
#include <iostream>
#include <algorithm>

class IRefCountable
{
	friend class RefBase;

public:
	virtual ~IRefCountable() = default;

	inline int GetRefCount() const
	{
		return m_Refrences;
	}

	inline bool InUse() const
	{
		return m_Refrences > 0;
	}

	inline void AddRef()
	{
		m_Refrences++;
	}

	inline void RemoveRef()
	{
		m_Refrences = std::max(m_Refrences - 1, 0);
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