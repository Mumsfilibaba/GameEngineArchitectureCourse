#include "LoaderCOLLADA.h"

IResource* LoaderCOLLADA::LoadFromDisk(const std::string& file)
{
	return nullptr;
}


IResource* LoaderCOLLADA::LoadFromMemory(void* data, size_t size)
{
	return nullptr;
}


size_t LoaderCOLLADA::WriteToBuffer(const std::string& file, void* buffer)
{
	return size_t();
}


template<typename T>
struct TArray
{
	std::string ID;
	size_t Count;
	std::vector<T> Data;
};
using FloatArray = TArray<float>;


struct Geometry
{
	std::string ID;
	std::string Name;
};


struct LibraryGeometry
{
	std::vector<Geometry> Geometries;
};


enum ETag : uint32_t
{
	TAG_UNKNOWN		= 0,
	TAG_FLOAT_ARRAY = 1
};


ETag SearchTag(const char* iter);


std::vector<GameMesh> LoaderCOLLADA::ReadFromDisk(const std::string& filepath)
{
	//Read in the full textfile into a buffer
	const char* buffer = nullptr;
	uint32_t bytes = ReadTextfile(filepath, &buffer);
	if (bytes == 0)
	{
		ThreadSafePrintf("Failed to load '%s'\n", filepath.c_str());
		return std::vector<GameMesh>();
	}
	
	FloatArray positions;
	const char* iter = buffer;
	while (*iter != '\0')
	{
		switch (*iter)
		{
		case '<':
		{
			ETag tag = SearchTag(iter);
		}
			break;
		default:
			break;
		}

		iter++;
	}

	return std::vector<GameMesh>();
}


ETag SearchTag(const char* iter)
{
	struct TagPair
	{
		ETag Tag			= TAG_UNKNOWN;
		const char* pTagStr = nullptr;
	};
	static const TagPair pairs[] =
	{
		{ TAG_FLOAT_ARRAY, "float_array" },
	};


	constexpr size_t numPairs = sizeof(pairs) / sizeof(TagPair);
	for (size_t i = 0; i < numPairs; i++)
	{
		const char* pRes = strstr(iter, pairs[i].pTagStr);

	}

	return TAG_UNKNOWN;
}
