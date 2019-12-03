#pragma once

class IRefCountable
{
	friend class RefBase;

public:
	virtual ~IRefCountable() = default;

protected:
	inline IRefCountable() : m_Refrences(0) {};

private:
	int m_Refrences;
};