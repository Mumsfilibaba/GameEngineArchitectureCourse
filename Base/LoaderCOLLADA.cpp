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


ETag SearchTag(const char** iter);
const char* GetEndOfString(const char** iter);

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
	
	std::vector<FloatArray> positions;
	const char* iter = buffer;
	while (*iter != '\0')
	{
		switch (*iter)
		{
		case '<':
		{
			iter++;
			if (*(iter) == '/')
			{
				break;
			}

			ETag tag = SearchTag(&iter);
			if (tag == TAG_FLOAT_ARRAY)
			{
				if (*iter != ' ')
				{
					//TODO: Display error
					break;
				}

				iter++;
				int result = strncmp(iter, "id=\"", 4);
				if (result == 0)
				{
					iter += 4;
				}
				else
				{
					//TODO: Display error
					break;
				}

				FloatArray arr;
				const char* idBegin = iter;
				const char* idEnd = GetEndOfString(&iter);
				arr.ID = std::string(idBegin, size_t(idEnd - idBegin));

				if (*iter != ' ')
				{
					//TODO: Display error
					break;
				}

				positions.emplace_back(arr);
			}

			break;
		}
		default:
			break;
		}

		iter++;
	}

	return std::vector<GameMesh>();
}


inline ETag SearchTag(const char** iter)
{
	//Create pairs of tags we are interested in
	struct TagPair
	{
		ETag Tag = TAG_UNKNOWN;
		const char* pTagStr = nullptr;
		uint32_t TagLength = 0;
	};
	static const TagPair pairs[] =
	{
		{ TAG_FLOAT_ARRAY, "float_array", strlen("float_array") }
	};

	//Check if the tag is some of the pairs
	constexpr size_t numPairs = sizeof(pairs) / sizeof(TagPair);
	for (size_t i = 0; i < numPairs; i++)
	{
		int result = strncmp(pairs[i].pTagStr, *iter, pairs[i].TagLength);
		if (result == 0)
		{
			(*iter) += pairs[i].TagLength;
			return pairs[i].Tag;
		}
	}
	return TAG_UNKNOWN;
}

inline const char* GetEndOfString(const char** iter)
{	
	while ((**iter) != '\"')
		(*iter)++;

	return *iter;
}
