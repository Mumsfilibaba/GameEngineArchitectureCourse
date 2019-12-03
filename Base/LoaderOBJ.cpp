#include "LoaderOBJ.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "StackAllocator.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4201)		//Disable: "nonstandard extension used: nameless struct/union"-warning
#endif

#define OBJ_ERROR_SUCCESS		0
#define OBJ_ERROR_CORRUPT_FILE	1
#define OBJ_ERROR_EMPTY_FILE	2

//#define DEBUG_PRINTS			1

struct OBJVertex
{
	int32_t Position = -1;
	int32_t Normal	 = -1;
	int32_t TexCoord = -1;

	inline bool operator==(const OBJVertex& other) const
	{
		return (Position == other.Position) && (Normal == other.Normal) && (TexCoord == other.TexCoord);
	}

	inline bool operator!=(const OBJVertex& other) const
	{
		return !(*this == other);
	}
};

struct OBJData
{
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec2> TextureCoords;
};
    
struct BinaryMeshData
{
    uint32_t VertexCount    = 0;
    uint32_t IndexCount     = 0;
};

using OBJMesh = TMesh<OBJVertex>;

bool LoadOBJ(std::vector<GameMesh>& meshes, const std::string& filepath);
uint32_t ReadTextfile(const std::string& filename, const char** const buffer);
void PrintError(int32_t error);
void SkipLine(const char** const iter);
void ParseOBJVertex(OBJVertex& vertex, const char** const buffer);
void ParseVec2(glm::vec2& vector, const char** const iter);
void ParseVec3(glm::vec3& vector, const char** const iter);
void AddUniqueOBJVertex(const OBJVertex& vertex, OBJMesh& mesh);


IResource* LoaderOBJ::LoadFromDisk(const std::string& file)
{
    std::vector<GameMesh> meshes = std::move(ReadFromDisk(file));
    if (!meshes.empty())
    {
        //Create mesh object
        auto& vertices = meshes[0].Vertices;
        auto& indices  = meshes[0].Indices;

        //Create a new array for these (That can be stores inside the Mesh and destroyed when constructed later)
        Vertex* pVertices = new(mm_allocate(sizeof(Vertex) * vertices.size(), 1, file + "Vertices")) Vertex[vertices.size()];
        memcpy(pVertices, vertices.data(), sizeof(Vertex) * vertices.size());

        uint32_t* pIndices = new(mm_allocate(sizeof(uint32_t) * indices.size(), 1, file + "Indices")) uint32_t[indices.size()];
        memcpy(pIndices, indices.data(), sizeof(uint32_t) * indices.size());

        return new(file.c_str()) Mesh(pVertices, pIndices, uint32_t(vertices.size()), uint32_t(indices.size()));
    }

    return nullptr;
}


IResource* LoaderOBJ::LoadFromMemory(void* pData, size_t)
{
    if (pData)
    {
        BinaryMeshData data = {};
        memcpy(&data, pData, sizeof(BinaryMeshData));
        
        uint8_t* pByteBuffer = (uint8_t*)pData;
        
        //Create a new array for these (That can be stores inside the Mesh and destroyed when constructed later)
        uint32_t vertexStride = sizeof(Vertex) * data.VertexCount;
        Vertex* pVertices = new(mm_allocate(vertexStride, 1, "Vertices LoadFromMemory")) Vertex[data.VertexCount];
        memcpy(pVertices, pByteBuffer + sizeof(BinaryMeshData), vertexStride);

        uint32_t indexStride = sizeof(uint32_t) * data.IndexCount;
        uint32_t* pIndices = new(mm_allocate(indexStride, 1, "Indices LoadFromMemory")) uint32_t[data.IndexCount];
        memcpy(pIndices, pByteBuffer + sizeof(BinaryMeshData) + vertexStride, indexStride);
        
        return new("Mesh LoadedFromMemory") Mesh(pVertices, pIndices, data.VertexCount, data.IndexCount);
    }
    
    return nullptr;
}


size_t LoaderOBJ::WriteToBuffer(const std::string& file, void* buffer)
{
    std::vector<GameMesh> meshes = std::move(ReadFromDisk(file));
    if (!meshes.empty())
    {
        BinaryMeshData data = {};
        data.VertexCount    = uint32_t(meshes[0].Vertices.size());
        data.IndexCount     = uint32_t(meshes[0].Indices.size());
        
        memcpy(buffer, &data, sizeof(BinaryMeshData));
        
        uint8_t* pByteBuffer = (uint8_t*)buffer;
        uint32_t vertexStride = sizeof(Vertex) * data.VertexCount;
        memcpy(pByteBuffer + sizeof(BinaryMeshData), meshes[0].Vertices.data(), vertexStride);
        
        uint32_t indexStride = sizeof(uint32_t) * data.IndexCount;
        memcpy(pByteBuffer + sizeof(BinaryMeshData)+ vertexStride, meshes[0].Indices.data(), indexStride);
        
        return sizeof(BinaryMeshData) + vertexStride + indexStride;
    }
    
	return size_t(-1);
}


std::vector<GameMesh> LoaderOBJ::ReadFromDisk(const std::string& filepath)
{
	std::vector<GameMesh> meshes;
	if (!::LoadOBJ(meshes, filepath))
	{
        ThreadSafePrintf("Failed to load mesh '%s'\n", filepath.c_str());
	}

	return meshes;
}


inline void SkipLine(const char** const iter)
{
	//Uses space since all other characters aren't relevant (See ASCII-table)
	while ((**iter) >= ' ')
		(*iter)++;
}


inline void ParseVec2(glm::vec2& vector, const char** const iter)
{
	int32_t length = 0;
	vector.x = (float)FastAtof(*iter, length);
	(*iter) += length + 1;

	vector.y = (float)FastAtof(*iter, length);
	(*iter) += length;
}


inline void ParseVec3(glm::vec3& vector, const char** const iter)
{
	int32_t length = 0;
	vector.x = (float)FastAtof(*iter, length);
	(*iter) += length + 1;

	vector.y = (float)FastAtof(*iter, length);
	(*iter) += length + 1;

	vector.z = (float)FastAtof(*iter, length);
	(*iter) += length;
}


inline void AddUniqueOBJVertex(const OBJVertex& vertex, OBJMesh& mesh)
{
	for (size_t i = 0; i < mesh.Vertices.size(); i++)
	{
		if (mesh.Vertices[i] == vertex)
		{
			mesh.Indices.emplace_back(uint32_t(i));
			return;
		}
	}

	mesh.Vertices.emplace_back(vertex);
	mesh.Indices.emplace_back(uint32_t(mesh.Vertices.size() - 1));
}


inline void ParseOBJVertex(OBJVertex& vertex, const char** const iter)
{
	int32_t length = 0;
	vertex.Position = FastAtoi((*iter), length);
	(*iter) += (length + 1);

	vertex.TexCoord = FastAtoi((*iter), length);
	(*iter) += (length + 1);

	vertex.Normal = FastAtoi((*iter), length);
	(*iter) += length;
}


inline void PrintError(int32_t error)
{
	switch (error)
	{
	case OBJ_ERROR_EMPTY_FILE:
		ThreadSafePrintf("ERROR LOADING OBJ: File is empty\n");
		break;
	case OBJ_ERROR_CORRUPT_FILE:
		ThreadSafePrintf("ERROR LOADING OBJ: Corrupt file\n");
		break;
	case OBJ_ERROR_SUCCESS:
	default:
		ThreadSafePrintf("LOADED OBJ SUCCESSFULLY\n");
		break;
	}
}


inline bool LoadOBJ(std::vector<GameMesh>& meshes, const std::string& filepath)
{
	using namespace glm;

	//Read in the full textfile into a buffer
	const char* buffer = nullptr;
	uint32_t bytes = ReadTextfile(filepath, &buffer);
	if (bytes == 0)
	{
		return false;
	}

	//OBJ-data
	OBJData fileData;
	std::vector<OBJMesh> objMeshes;

	//Declare variables here to save on allocations
	vec3 vec3;
	vec2 vec2;
	OBJVertex vertex;
	int32_t i = 0;

	//Init mesh
	objMeshes.resize(1);

	//Start parsing
	uint32_t currentMesh = 0;
	const char* iter = buffer;
	while (*iter != '\0')
	{
		switch (*iter)
		{
		case '#':
			//Comment
			SkipLine(&iter);
			break;
		case 'v':
			//Some vertex attribute
			iter++;

			if (*iter == 't')
			{
				//TextureCoords
				iter++;
				if ((*iter) != ' ')
				{
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return false;
				}

				//Parse TextureCoords
				iter++;
				ParseVec2(vec2, &iter);

				if ((*iter) != '\n')
				{
					//After two floats there should be a new line otherwise a corrupt file
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return false;
				}

				fileData.TextureCoords.emplace_back(vec2);
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("Found TexCoord: %s\n", to_string(vec2).c_str());
#endif
			}
			else if ((*iter) == 'n')
			{
				//Normals
				iter++;
				if ((*iter) != ' ')
				{
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return false;
				}

				//Parse Normal
				iter++;
				ParseVec3(vec3, &iter);

				if ((*iter) != '\n')
				{
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return false;
				}

				fileData.Normals.emplace_back(vec3);
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("Found Normal: %s\n", to_string(vec3).c_str());
#endif
			}
			else if ((*iter) == ' ')
			{
				//Parse Normal
				iter++;
				ParseVec3(vec3, &iter);

				if ((*iter) != '\n')
				{
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					free((void*)buffer);
					return false;
				}

				fileData.Positions.emplace_back(vec3);
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("Found Position: %s\n", glm::to_string(vec3).c_str());
#endif
			}
			else
			{
				//The character after v is not a 't', 'n' or ' ' we have a corrupted file
				PrintError(OBJ_ERROR_CORRUPT_FILE);

				free((void*)buffer);
				return false;
			}

			break;
		case 'f':
#if defined(DEBUG_PRINTS)
			ThreadSafePrintf("Found face: ");
#endif
			iter++;

			//If there is not a blank space here the file is not valid and we return
			if ((*iter) != ' ')
			{
				PrintError(OBJ_ERROR_CORRUPT_FILE);

				free((void*)buffer);
				return false;
			}

			//Read in three vertices
			for (i = 0; i < 3; i++)
			{
				iter++;
				ParseOBJVertex(vertex, &iter);
				AddUniqueOBJVertex(vertex, objMeshes[currentMesh]);
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("%d/%d/%d ", vertex.Position, vertex.TexCoord, vertex.Normal);
#endif
			}

			//Check if we have a polygon and convert it to triangles
			{
                size_t numIndices   = objMeshes[currentMesh].Indices.size();
				uint32_t startIndex = objMeshes[currentMesh].Indices[numIndices - 3]; //Get the first vertex in the base triangle
				while ((*iter) != '\n' && (*iter) != '\0')
				{
					iter++;
					ParseOBJVertex(vertex, &iter);

					//Add a new triangle base on the last couple of ones
					objMeshes[currentMesh].Indices.emplace_back(startIndex);
					objMeshes[currentMesh].Indices.emplace_back(objMeshes[currentMesh].Indices[numIndices - 1]);

					AddUniqueOBJVertex(vertex, objMeshes[currentMesh]);
#if defined(DEBUG_PRINTS)
					ThreadSafePrintf("%d/%d/%d ", vertex.Position, vertex.TexCoord, vertex.Normal);
#endif
				}
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("\n");
#endif
				}
			break;
		case 'g':
#if defined(DEBUG_PRINTS)
			ThreadSafePrintf("Found group\n");
#endif
			break;
		default:
			SkipLine(&iter);
			break;
		}

		iter++;
	}

#if defined(DEBUG_PRINTS)
	//Print all the vertices and indicess
	ThreadSafePrintf("Vertices:\n");
	for (int i = 0; i < objMeshes[0].Vertices.size(); i++)
	{
		ThreadSafePrintf("%d/%d/%d\n", objMeshes[0].Vertices[i].Position, objMeshes[0].Vertices[i].TexCoord, objMeshes[0].Vertices[i].Normal);
	}

	ThreadSafePrintf("Indices:\n");
	for (int i = 0; i < objMeshes[0].Indices.size(); i++)
	{
		ThreadSafePrintf("%d\n", objMeshes[0].Indices[i]);
	}
#endif

	//Convert all the OBJVertices to vertioces that the game can use
	for (auto& mesh : objMeshes)
	{
		//Create a new gamemesh
		meshes.emplace_back();

		//Convert all vertices
		Vertex v = {};
		for (auto& vertex : mesh.Vertices)
		{
			v.Position	= vertex.Position > 0	? fileData.Positions[vertex.Position - 1] : glm::vec3();
			v.Normal	= vertex.Normal > 0		? fileData.Normals[vertex.Normal - 1] : glm::vec3();
			v.TexCoords = vertex.TexCoord > 0	? fileData.TextureCoords[vertex.TexCoord - 1] : glm::vec2();
			meshes.back().Vertices.emplace_back(v);
		}

		meshes.back().Indices.swap(mesh.Indices);
	}

	free((void*)buffer);
	return true;
}
