#pragma once
#include <iostream>

class IRefCountable
{
	friend class RefBase;

public:
	virtual ~IRefCountable() = default;

protected:
	inline IRefCountable() : m_Refrences(0) {};

	inline virtual void InternalRelease()
	{
		delete this;
	};

	inline void AddRef()
	{
		m_Refrences++;
	}

	inline void RemoveRef()
	{
		if (--m_Refrences == 0)
			InternalRelease();
	}

private:
	int m_Refrences;
};