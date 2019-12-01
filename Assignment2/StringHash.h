#ifndef STRING_HASH_H
#define STRING_HASH_H

constexpr unsigned long long PRIME_MULTIPLE = 16777619ull;
constexpr unsigned int INITIAL_HASH = 2166136261u;

template <size_t index>
class Hash
{
public:
	template <size_t strLen>
	constexpr static unsigned int Generate(const char(&str)[strLen])
	{
        using THash = Hash<index - 1u>;
		return static_cast<unsigned int>( static_cast<unsigned long long>(THash::Generate(str) ^ (unsigned int)(str[index - 1u])) * PRIME_MULTIPLE);
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

#endif
