#include "LoaderOBJ.h"
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "StackAllocator.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4201)		//Disable: "nonstandard extension used: nameless struct/union"-warning
#endif

#define OBJ_ERROR_SUCCESS				0
#define OBJ_ERROR_CORRUPT_FILE			1
#define OBJ_ERROR_EMPTY_FILE			2
#define OBJ_ERROR_INDICES_OUT_OF_BOUNDS 3

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

bool LoadOBJ(std::vector<MeshData>& meshes, const std::string& filepath);
uint32_t ReadTextfile(const std::string& filename, const char** const buffer);
void PrintError(int32_t error);
void SkipLine(const char** const iter);
void ParseOBJVertex(OBJVertex& vertex, const char** const buffer);
void ParseVec2(glm::vec2& vector, const char** const iter);
void ParseVec3(glm::vec3& vector, const char** const iter);
void AddUniqueOBJVertex(const Vertex& vertex, std::unordered_map<Vertex, uint32_t>& uniqueVertices, MeshData& mesh);


IResource* LoaderOBJ::LoadFromDisk(const std::string& file)
{
    std::vector<MeshData> meshes = ReadFromDisk(file);
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
    std::vector<MeshData> meshes = ReadFromDisk(file);
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


std::vector<MeshData> LoaderOBJ::ReadFromDisk(const std::string& filepath)
{
	std::vector<MeshData> meshes;
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


inline void AddUniqueOBJVertex(const Vertex& vertex, std::unordered_map<Vertex, uint32_t>& uniqueVertices, MeshData& mesh)
{
	if (uniqueVertices.count(vertex) == 0)
	{
		uniqueVertices[vertex] = (uint32_t)mesh.Vertices.size();
		mesh.Vertices.push_back(vertex);
	}

	mesh.Indices.emplace_back(uniqueVertices[vertex]);
}


inline void ParseOBJVertex(OBJVertex& objVertex, const char** const iter)
{
	int32_t length = 0;
	objVertex.Position = FastAtoi((*iter), length);
	(*iter) += (length + 1);

	objVertex.TexCoord = FastAtoi((*iter), length);
	(*iter) += (length + 1);

	objVertex.Normal = FastAtoi((*iter), length);
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
	case OBJ_ERROR_INDICES_OUT_OF_BOUNDS:
		ThreadSafePrintf("ERROR LOADING OBJ: Indices out of bounds\n");
		break;
	case OBJ_ERROR_SUCCESS:
	default:
		ThreadSafePrintf("LOADED OBJ SUCCESSFULLY\n");
		break;
	}
}


inline bool IsNewLine(const char** const iter)
{
	if ((*(*iter) == '\n'))
	{
		return true;
	}
	else if (*(*iter) == '\r')
	{
		(*iter)++;
		if ((*(*iter) == '\n'))
		{
			return true;
		}
	}

	return false;
}


inline bool LoadOBJ(std::vector<MeshData>& meshes, const std::string& filepath)
{
	using namespace glm;

	//Read in the full textfile into a buffer
	const char* buffer = nullptr;
	uint32_t bytes = ReadTextfile(filepath, &buffer);
	if (bytes == 0)
	{
		ThreadSafePrintf("ERROR LOADING OBJ: Failed to load file\n");
		return false;
	}

	//Vertexdata
	Vertex vertex;
	OBJVertex objVertex;
	OBJData filedata;
	std::unordered_map<Vertex, uint32_t> uniqueVertices;
	meshes.resize(1);

	//Declare variables here to save on allocations
	vec3 vec3;
	vec2 vec2;
	int32_t i = 0;

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
			//Parse vertex attribute
			iter++;

			//TextureCoords
			if (*iter == 't')
			{
				iter++;
				if ((*iter) != ' ')
				{
					//There should be an empty space after the attribute
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					mm_free((void*)buffer);
					return false;
				}

				//Parse TextureCoords
				iter++;
				ParseVec2(vec2, &iter);

				if (!IsNewLine(&iter))
				{
					//After two floats there should be a new line otherwise a corrupt file
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					mm_free((void*)buffer);
					return false;
				}

				filedata.TextureCoords.emplace_back(vec2);
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

					mm_free((void*)buffer);
					return false;
				}

				//Parse Normal
				iter++;
				ParseVec3(vec3, &iter);

				if (!IsNewLine(&iter))
				{
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					mm_free((void*)buffer);
					return false;
				}

				filedata.Normals.emplace_back(vec3);
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("Found Normal: %s\n", to_string(vec3).c_str());
#endif
			}
			else if ((*iter) == ' ')
			{
				//Parse Normal
				iter++;
				ParseVec3(vec3, &iter);

				if (!IsNewLine(&iter))
				{
					PrintError(OBJ_ERROR_CORRUPT_FILE);

					mm_free((void*)buffer);
					return false;
				}

				filedata.Positions.emplace_back(vec3);
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("Found Position: %s\n", glm::to_string(vec3).c_str());
#endif
			}
			else
			{
				//The character after v is not a 't', 'n' or ' ' we have a corrupted file
				PrintError(OBJ_ERROR_CORRUPT_FILE);

				mm_free((void*)buffer);
				return false;
			}

			break;
		case 'f':
			//Parse a face
			iter++;

#if defined(DEBUG_PRINTS)
			ThreadSafePrintf("Found face: ");
#endif

			//If there is not a blank space here the file is not valid and we return
			if ((*iter) != ' ')
			{
				PrintError(OBJ_ERROR_CORRUPT_FILE);

				mm_free((void*)buffer);
				return false;
			}

			//Read in three vertices
			for (i = 0; i < 3; i++)
			{
				iter++;

				//Get vertex
				ParseOBJVertex(objVertex, &iter);
				if ((*iter) == '\0')
					break;

				//Construct and insert new vertex
				if ((objVertex.Position > 0) && (objVertex.Normal > 0) && (objVertex.TexCoord > 0))
				{
					vertex.Position = filedata.Positions[objVertex.Position - 1];
					vertex.Normal = filedata.Normals[objVertex.Normal - 1];
					vertex.TexCoords = filedata.TextureCoords[objVertex.TexCoord - 1];
					AddUniqueOBJVertex(vertex, uniqueVertices, meshes[currentMesh]);
				}
				else
				{
					PrintError(OBJ_ERROR_INDICES_OUT_OF_BOUNDS);

					mm_free((void*)buffer);
					return false;
				}
#if defined(DEBUG_PRINTS)
				ThreadSafePrintf("%d/%d/%d ", objVertex.Position, objVertex.TexCoord, objVertex.Normal);
#endif
			}

			//Check if we have a polygon and convert it to triangles
			{
                size_t numIndices   = meshes[currentMesh].Indices.size();
				uint32_t startIndex = meshes[currentMesh].Indices[numIndices - 3]; //Get the first vertex in the base triangle
				while (!IsNewLine(&iter))
				{
					iter++;
					//Get vertex
					ParseOBJVertex(objVertex, &iter);
					if ((*iter) == '\0')
						break;
					
					//Add a new triangle base on the last couple of ones
					meshes[currentMesh].Indices.emplace_back(startIndex);
					meshes[currentMesh].Indices.emplace_back(meshes[currentMesh].Indices[numIndices - 1]);

					//Construct and insert new vertex
					if ((objVertex.Position > 0) && (objVertex.Normal > 0) && (objVertex.TexCoord > 0))
					{
						vertex.Position		= filedata.Positions[objVertex.Position - 1];
						vertex.Normal		= filedata.Normals[objVertex.Normal - 1];
						vertex.TexCoords	= filedata.TextureCoords[objVertex.TexCoord - 1];
						AddUniqueOBJVertex(vertex, uniqueVertices, meshes[currentMesh]);
					}
					else
					{
						PrintError(OBJ_ERROR_INDICES_OUT_OF_BOUNDS);

						mm_free((void*)buffer);
						return false;
					}
#if defined(DEBUG_PRINTS)
					ThreadSafePrintf("%d/%d/%d ", objVertex.Position, objVertex.TexCoord, objVertex.Normal);
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
	for (int i = 0; i < meshes[0].Vertices.size(); i++)
	{
		ThreadSafePrintf("%d/%d/%d\n", meshes[0].Vertices[i].Position, meshes[0].Vertices[i].TexCoords, meshes[0].Vertices[i].Normal);
	}

	ThreadSafePrintf("Indices:\n");
	for (int i = 0; i < meshes[0].Indices.size(); i++)
	{
		ThreadSafePrintf("%d\n", meshes[0].Indices[i]);
	}
#endif
    
    //Print number of vertices, indices and triangles
    ThreadSafePrintf("Finished loading OBJ-file '%s' - VertexCount=%d, IndexCount=%d, TriangleCount=%d\n", filepath.c_str(), meshes[0].Vertices.size(), meshes[0].Indices.size(), meshes[0].Indices.size() / 3);

	mm_free((void*)buffer);
	return true;
}
