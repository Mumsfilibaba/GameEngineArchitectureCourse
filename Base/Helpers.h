#pragma once
#include "Defines.h"
#include <string>
#include <cstdint>
#include <vector>
#include <SFML/System/Time.hpp>
#include <glad/glad.h>
#include <cassert>

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4505)		//Disable: "unreferenced local function has been removed"-warning
	#pragma warning(disable : 4201)		//Disable: "nonstandard extension used: nameless struct/union"-warning
#endif

#if defined(_DEBUG)
	//#define GL_DEBUG_ASSERT
#endif

bool GLHasErrors();
void ClearErrors();

#if defined(GL_DEBUG_ASSERT)
	#define GL_CALL(x) ClearErrors(); x; assert(GLHasErrors() == false)
#else
	#define GL_CALL(x) x
#endif

constexpr uint64_t PRIME_MULTIPLE	= 16777619ull;
constexpr uint32_t INITIAL_HASH		= 2166136261u;

std::string N2HexStr(size_t w);

//IMGUI FUNCTIONS 
void ImGuiDrawMemoryProgressBar(size_t used, size_t available);
void ImGuiPrintMemoryManagerAllocations();
void ImGuiDrawFrameTimeGraph(const sf::Time& deltatime);

void ThreadSafePrintf(const char* pFormat, ...);

//PARSING FUNCTIONS 
double FastAtof(const char* const str, int32_t& length);
int32_t FastAtoi(const char* const str, int32_t& length);
uint32_t ReadTextfile(const std::string& filename, const char** const ppBuffer);

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
		{
			hash = (*str ^ hash) * PRIME_MULTIPLE;
			str++;
		}

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


template<typename T>
inline void HashCombine(size_t& hash, const T& value)
{
	std::hash<T> hasher;
	hash ^= hasher(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}
