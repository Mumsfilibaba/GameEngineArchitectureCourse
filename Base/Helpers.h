#pragma once
#include "Defines.h"
#include <string>
#include <cstdint>
#include <SFML/System/Time.hpp>

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4505)		//Disable: "unreferenced local function has been removed"-warning
#endif

constexpr uint64_t PRIME_MULTIPLE	= 16777619ull;
constexpr uint32_t INITIAL_HASH		= 2166136261u;

std::string N2HexStr(size_t w);

//IMGUI FUNCTIONS 
void ImGuiDrawMemoryProgressBar(int32_t used, int32_t available);
void ImGuiPrintMemoryManagerAllocations();
void ImGuiDrawFrameTimeGraph(const sf::Time& deltatime);

void ThreadSafePrintf(const char* pFormat, ...);

//PARSING FUNCTIONS 
double FastAtof(const char* const str, int32_t& length);
int32_t FastAtoi(const char* const str, int32_t& length);


//RAND FUNCTIONS 
static float randf()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

static float randf(float min, float max)
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX)* (max - min) + min;
}


//HASHING FUNCTIONS 
template <size_t index>
class Hash
{
public:
	template <size_t strLen>
	constexpr static unsigned int Generate(const char(&str)[strLen])
	{
		using THash = Hash<index - 1u>;
		return static_cast<unsigned int>(static_cast<unsigned long long>(THash::Generate(str) ^ (unsigned int)(str[index - 1u]))* PRIME_MULTIPLE);
	}
};

template <>
class Hash<0u>
{
public:
	template <size_t strLen>
	constexpr static unsigned int Generate(const char(&str)[strLen])
	{
		return INITIAL_HASH;
	}
};

template <typename  T>
class HashHelper {};

template <>
class HashHelper<const char*>
{
private:
	static unsigned int Fnv1aHash(const char* str, unsigned int hash = INITIAL_HASH)
	{
		while (*str != 0)
			hash = (*str ^ hash) * PRIME_MULTIPLE;

		return hash;
	}

public:
	static unsigned int Generate(const char* str)
	{
		return Fnv1aHash(str);
	}
};

template <size_t strLen>
class HashHelper<char[strLen]>
{
public:
	static constexpr unsigned int Generate(const char(&str)[strLen])
	{
		return Hash<strLen - 1u>::Generate(str);
	}
};


template <class Type>
constexpr unsigned int HashString(const Type& str)
{
	return HashHelper<Type>::Generate(str);
}
