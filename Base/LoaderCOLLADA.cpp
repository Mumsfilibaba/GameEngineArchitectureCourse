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



#define COLLADA_ERROR_SUCCESS				0
#define COLLADA_ERROR_CORRUPT_FILE			1
#define COLLADA_ERROR_EMPTY_FILE			2
#define COLLADA_ERROR_INDICES_OUT_OF_BOUNDS 3

inline void PrintError(int32_t error)
{
	switch (error)
	{
	case COLLADA_ERROR_EMPTY_FILE:
		ThreadSafePrintf("ERROR LOADING OBJ: File is empty\n");
		break;
	case COLLADA_ERROR_CORRUPT_FILE:
		ThreadSafePrintf("ERROR LOADING OBJ: Corrupt file\n");
		break;
	case COLLADA_ERROR_INDICES_OUT_OF_BOUNDS:
		ThreadSafePrintf("ERROR LOADING OBJ: Indices out of bounds\n");
		break;
	case COLLADA_ERROR_SUCCESS:
	default:
		ThreadSafePrintf("LOADED OBJ SUCCESSFULLY\n");
		break;
	}
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


struct TagData
{
	std::string ID;
	std::unordered_map<std::string, std::string> Data;
};


enum ETag : uint32_t
{
	TAG_UNKNOWN		= 0,
	TAG_FLOAT_ARRAY = 1
};


bool IsTagTag(const char** iter, const char* tagName);
TagData SearchTag(const char** buffer, const char* tagName);
const char* GetEndOfString(const char** iter);

std::vector<MeshData> LoaderCOLLADA::ReadFromDisk(const std::string& filepath)
{
	//Read in the full textfile into a buffer
	const char* buffer = nullptr;
	uint32_t bytes = ReadTextfile(filepath, &buffer);
	if (bytes == 0)
	{
		ThreadSafePrintf("Failed to load '%s'\n", filepath.c_str());
		return std::vector<MeshData>();
	}
	
	const char* iter = buffer;
	TagData libraryGeometries	= SearchTag(&iter, "library_geometries");
	TagData geometries			= SearchTag(&iter, "geometry");

	free((void*)buffer);
	return std::vector<MeshData>();
}


inline TagData SearchTag(const char** buffer, const char* tagName)
{
	TagData data;

	//std::vector<FloatArray> positions;
	const char* iter = (*buffer);
	while (*iter != '\0')
	{
		if (*iter == '<')
		{
			iter++;
			if (*(iter) == '/')
			{
				continue;
			}

			//Check if this is correct tag
			if (!IsTagTag(&iter, tagName))
			{
				continue;
			}
			
			ThreadSafePrintf("Found %s\n", tagName);

			//Did we reach endtag
			data.ID = tagName;
			if ((*iter) == '>')
				break;

			//Find all values for this tag
			//while (*iter >= 'a' && *iter <= 'z')
				//iter++;

			/*if (tag == TAG_FLOAT_ARRAY)
			{
				if (*iter != ' ')
				{
					PrintError(COLLADA_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return;
				}

				//Parse ID 
				iter++;
				int result = strncmp(iter, "id=\"", 4);
				if (result == 0)
				{
					iter += 4;
				}
				else
				{
					PrintError(COLLADA_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return;
				}

				FloatArray arr;
				const char* idBegin = iter;
				const char* idEnd = GetEndOfString(&iter);
				arr.ID = std::string(idBegin, size_t(idEnd - idBegin));

				//Parse count
				iter++;
				if (*iter != ' ')
				{
					PrintError(COLLADA_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return;
				}

				iter++;
				result = strncmp(iter, "count=\"", 7);
				if (result == 0)
				{
					iter += 7;
				}
				else
				{
					PrintError(COLLADA_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return;
				}

				int32_t length = 0;
				arr.Count = FastAtoi(iter, length);

				//Parse data
				iter += length + 2;
				for (uint32_t i = 0; i < arr.Count; i++)
				{
					float d = (float)FastAtof(iter, length);
					if (length == 0)
					{
						PrintError(COLLADA_ERROR_CORRUPT_FILE);

						free((void*)buffer);
						return;
					}

					iter += length + 1;
					arr.Data.emplace_back(d);
				}

				positions.emplace_back(arr);
			}*/
		}

		iter++;
	}

	//Return the new iterator
	(*buffer) = iter;
	return data;
}


inline bool IsTagTag(const char** iter, const char* tagName)
{
	//Check if the tag is correct
	int length = strlen(tagName);
	int result = strncmp(tagName, *iter, length);
	if (result == 0)
	{
		(*iter) += length;
		return true;
	}

	return false;
}


inline const char* GetEndOfString(const char** iter)
{	
	while ((**iter) != '\"')
		(*iter)++;

	return *iter;
}
