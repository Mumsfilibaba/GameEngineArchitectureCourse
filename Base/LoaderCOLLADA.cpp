#include "LoaderCOLLADA.h"
#include "StackAllocator.h"
#include <tinyxml2.h>
#include <string>
#include <unordered_map>

#define DEBUG_PRINTS 0
#define MAX_STRING_LENGTH 128

IResource* LoaderCOLLADA::LoadFromDisk(const std::string& file)
{
    MeshData mesh = ReadFromDisk(file);
    if (!mesh.Vertices.empty())
    {
        //Create mesh object
        auto& vertices = mesh.Vertices;
        auto& indices  = mesh.Indices;

        //Create a new array for these (That can be stores inside the Mesh and destroyed when constructed later)
        Vertex* pVertices = new(mm_allocate(sizeof(Vertex) * vertices.size(), 1, file + "Vertices")) Vertex[vertices.size()];
        memcpy(pVertices, vertices.data(), sizeof(Vertex) * vertices.size());

        uint32_t* pIndices = new(mm_allocate(sizeof(uint32_t) * indices.size(), 1, file + "Indices")) uint32_t[indices.size()];
        memcpy(pIndices, indices.data(), sizeof(uint32_t) * indices.size());

        return new(file.c_str()) Mesh(pVertices, pIndices, uint32_t(vertices.size()), uint32_t(indices.size()));
    }

    return nullptr;
}


IResource* LoaderCOLLADA::LoadFromMemory(void* pData, size_t)
{
    if (pData)
    {
        BinaryMeshData data = {};
        memcpy(&data, pData, sizeof(BinaryMeshData));
        
        uint8_t* pByteBuffer = (uint8_t*)pData;
        
        //Create a new array for these (That can be stores inside the Mesh and destroyed when constructed later)
        uint32_t vertexStride = sizeof(Vertex) * data.VertexCount;
        Vertex* pVertices = (Vertex*)mm_allocate(vertexStride, 1, "Vertices LoadFromMemory");
        memcpy(pVertices, pByteBuffer + sizeof(BinaryMeshData), vertexStride);

        uint32_t indexStride = sizeof(uint32_t) * data.IndexCount;
        uint32_t* pIndices = (uint32_t*)mm_allocate(indexStride, 1, "Indices LoadFromMemory");
        memcpy(pIndices, pByteBuffer + (sizeof(BinaryMeshData) + vertexStride), indexStride);
        
        return new("Mesh LoadedFromMemory") Mesh(pVertices, pIndices, data.VertexCount, data.IndexCount);
    }
    
    return nullptr;
}


size_t LoaderCOLLADA::WriteToBuffer(const std::string& file, void* buffer)
{
    MeshData mesh = ReadFromDisk(file);
    if (!mesh.Vertices.empty())
    {
        BinaryMeshData data = {};
        data.VertexCount    = uint32_t(mesh.Vertices.size());
        data.IndexCount     = uint32_t(mesh.Indices.size());
        
        memcpy(buffer, &data, sizeof(BinaryMeshData));
        
        uint8_t* pByteBuffer = (uint8_t*)buffer;
        uint32_t vertexStride = sizeof(Vertex) * data.VertexCount;
        memcpy(pByteBuffer + sizeof(BinaryMeshData), mesh.Vertices.data(), vertexStride);
        
        uint32_t indexStride = sizeof(uint32_t) * data.IndexCount;
        memcpy(pByteBuffer + sizeof(BinaryMeshData)+ vertexStride, mesh.Indices.data(), indexStride);
        
        return sizeof(BinaryMeshData) + vertexStride + indexStride;
    }
    
    return size_t(-1);
}



template<typename T>
struct TArray
{
	char ID[MAX_STRING_LENGTH];
    T* pData = nullptr;
    int32_t Count = 0;
};
using COLLADAFloatArray = TArray<float>;
using COLLADAIntArray = TArray<int32_t>;


struct COLLADAInput
{
    char Semantic[MAX_STRING_LENGTH];
    char Source[MAX_STRING_LENGTH];
    int32_t Offset = -1;
};


struct COLLADAParam
{
    char Name[MAX_STRING_LENGTH];
    char Type[32];
};


struct COLLADAAccessor
{
    char Source[MAX_STRING_LENGTH];
    int32_t Count;
    int32_t Stride;
    std::vector<COLLADAParam> Params;
};


struct COLLADASource
{
    char ID[MAX_STRING_LENGTH];
    COLLADAFloatArray FloatArray;
    COLLADAAccessor Accessor;
};


struct COLLADAMesh
{
    glm::vec3* pPositions = nullptr;
    int32_t NumPositions = 0;
    glm::vec3* pNormals = nullptr;
    int32_t NumNormals = 0;
    glm::vec2* pTexCoords = nullptr;
    int32_t NumTexCoords = 0;
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    std::unordered_map<Vertex, uint32_t> UniqueVertices;
};


inline COLLADAIntArray ReadIndices(uint32_t count, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    COLLADAIntArray indices = {};
    XMLElement* pIndices = pParent->FirstChildElement("p");
    if (pIndices != nullptr)
    {
        indices.Count = count;
        indices.pData = (int32_t*)stack_allocate(sizeof(int32_t) * count, 1, "Indices");
        memset(indices.pData, 0, sizeof(int32_t) * count);

        const char* pText = pIndices->GetText();
        
        //Parse the float data
        int32_t length = 0;
        const char* pIter = pText;
        for (int32_t i = 0; i < indices.Count; i++)
        {
            indices.pData[i] = FastAtoi(pIter, length);
            
            pIter += length;
            if (*pIter == ' ')
                pIter++;
        }
    }
    
    return indices;
}


inline COLLADAIntArray ReadVCount(uint32_t count, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    COLLADAIntArray vcount = {};
    XMLElement* pVCount = pParent->FirstChildElement("vcount");
    if (pVCount != nullptr)
    {
        vcount.Count = count;
        vcount.pData = (int32_t*)stack_allocate(sizeof(int32_t) * count, 1, "vcount");
        memset(vcount.pData, 0, sizeof(int32_t) * count);

        const char* pText = pVCount->GetText();
        
        //Parse the float data
        int32_t length = 0;
        const char* pIter = pText;
        for (int32_t i = 0; i < vcount.Count; i++)
        {
            vcount.pData[i] = FastAtoi(pIter, length);
            pIter += length;
            if (*pIter == ' ')
                pIter++;
        }
    }
    
    return vcount;
}


inline void ReadInputs(std::unordered_map<std::string, COLLADAInput>& inputs, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    XMLElement* pInput = pParent->FirstChildElement("input");
    while (pInput != nullptr)
    {
        COLLADAInput input = {};
        
        //Get semantic
        const char* semantic = pInput->Attribute("semantic");
        assert(semantic != nullptr);
        strcpy(input.Semantic, semantic);
        
        //Get source
        const char* source = pInput->Attribute("source");
        assert(semantic != nullptr);
        strcpy(input.Source, source);
        
        //Get offset and set
        XMLError result = pInput->QueryIntAttribute("offset", &input.Offset);
        if (result != XML_SUCCESS)
            input.Offset = -1;
        
        //Go to next
        inputs.emplace(std::make_pair(input.Semantic, input));
        pInput = pInput->NextSiblingElement("input");
    }
}


inline void ConstructVec3(const std::string& semantic, glm::vec3** pVec, int32_t& numVectors, std::unordered_map<std::string, COLLADASource>& sources, std::unordered_map<std::string, COLLADAInput>& inputs)
{
    if (*pVec != nullptr && numVectors != 0)
    {
        //Assume that this vector already has been constructed
        return;
    }

    //Read positions and construct positions
    auto input = inputs.find(semantic);
    if (input != inputs.end())
    {
        //Get source ID, but remove the # in the beginning
        char sourceID[MAX_STRING_LENGTH];
        strcpy(sourceID, input->second.Source + 1);
        
        //Get the source and construct positions
        auto source = sources.find(sourceID);
        if (source != sources.end())
        {
            size_t componentCount = source->second.Accessor.Params.size();
            assert(componentCount == 3); // We only support vec3 here
            
            auto& data = source->second.FloatArray;
            numVectors = data.Count / 3;
            glm::vec3* pVectors = (glm::vec3*)stack_allocate(sizeof(glm::vec3)*numVectors, 1, "Vec3Array");           
            for (size_t i = 0; i < data.Count; i += 3)
            {
                int32_t index = i / 3;
                pVectors[index].x = data.pData[i+0];
                pVectors[index].y = data.pData[i+1];
                pVectors[index].z = data.pData[i+2];
            }

            (*pVec) = pVectors;
        }
        else
        {
            ThreadSafePrintf("Source '%s' in COLLADA file was not found\n", input->second.Source);
        }
    }
}


inline void ConstructVec2(const std::string& semantic, glm::vec2** pVec, int32_t& numVectors, std::unordered_map<std::string, COLLADASource>& sources, std::unordered_map<std::string, COLLADAInput>& inputs)
{
    if (*pVec != nullptr && numVectors != 0)
    {
        //Assume that this vector already has been constructed
        return;
    }

    //Read positions and construct positions
    auto input = inputs.find(semantic);
    if (input != inputs.end())
    {
        //Get source ID, but remove the # in the beginning
        char sourceID[MAX_STRING_LENGTH];
        strcpy(sourceID, input->second.Source + 1);
        
        //Get the source and construct positions
        auto source = sources.find(sourceID);
        if (source != sources.end())
        {
            uint32_t componentCount = uint32_t(source->second.Accessor.Params.size());
            assert(componentCount == 2); // We only support vec2 here
            
            auto& data = source->second.FloatArray;
            numVectors = data.Count / 3;
            glm::vec2* pVectors = (glm::vec2*)stack_allocate(sizeof(glm::vec2) * numVectors, 1, "Vec3Array");
            for (size_t i = 0; i < data.Count; i += 2)
            {
                int32_t index = i / 2;
                pVectors[index].x = data.pData[i + 0];
                pVectors[index].y = data.pData[i + 1];
            }

            (*pVec) = pVectors;
        }
        else
        {
            ThreadSafePrintf("Source '%s' in COLLADA file was not found\n", input->second.Source);
        }
    }
}


inline void ReadVertices(COLLADAMesh& mesh, std::unordered_map<std::string, COLLADASource>& sources, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    //Get vertices element
    XMLElement* pVertices = pParent->FirstChildElement("vertices");
    if (pVertices != nullptr)
    {
        std::unordered_map<std::string, COLLADAInput> inputs;
        //Get source
        ReadInputs(inputs, pVertices);
        
        //Construct positions
        ConstructVec3("POSITION", &mesh.pPositions, mesh.NumPositions, sources, inputs);
    }
    else
    {
        ThreadSafePrintf("COLLADA file does not contain any vertices-element\n");
    }
}


inline void InsertUniqueVertex(COLLADAMesh& mesh, Vertex& vertex)
{
    //Insert unique vertex
    if (mesh.UniqueVertices.count(vertex) == 0)
    {
        mesh.UniqueVertices[vertex] = (uint32_t)mesh.Vertices.size();
        mesh.Vertices.push_back(vertex);
    }

    mesh.Indices.emplace_back(mesh.UniqueVertices[vertex]);
}


inline void ReadTriangles(COLLADAMesh& mesh, std::unordered_map<std::string, COLLADASource>& sources, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    //Get triangles
    XMLElement* pTriangles = pParent->FirstChildElement("triangles");
    if (pTriangles != nullptr)
    {
		while (pTriangles != nullptr)
		{
			//Get input
			std::unordered_map<std::string, COLLADAInput> inputs;
			ReadInputs(inputs, pTriangles);
        
			//Get trianglecount
			int32_t triangleCount = 0;
			XMLError result = pTriangles->QueryIntAttribute("count", &triangleCount);
			assert(result == XML_SUCCESS);
        
			//Get the indices (Count of indices is 'elementsPerVertex * numberOfTriangles * 3')
			COLLADAIntArray indices = ReadIndices(uint32_t(inputs.size()) * uint32_t(triangleCount) * 3U, pTriangles);
			if (indices.pData == nullptr)
			{
				ThreadSafePrintf("COLLADA file does not contain any indices\n");
				return;
			}
        
			//Construct normals
			ConstructVec3("NORMAL", &mesh.pNormals, mesh.NumNormals, sources, inputs);
			//Construct texcoords
			ConstructVec2("TEXCOORD", &mesh.pTexCoords, mesh.NumTexCoords, sources, inputs);

			//Read normal components
			int32_t normalOffset = -1;
			{
				auto normal = inputs.find("NORMAL");
				if (normal != inputs.end())
					normalOffset = normal->second.Offset;
			}
			//Read texcoords
			int32_t texcoordOffset = -1;
			{
				auto texcoord = inputs.find("TEXCOORD");
				if (texcoord != inputs.end())
					texcoordOffset = texcoord->second.Offset;
			}
			//Read position offset
			int32_t positionOffset = -1;
			{
				auto position = inputs.find("VERTEX");
				if (position != inputs.end())
					positionOffset = position->second.Offset;
			}
             
			Vertex vertex = {};
			size_t vertexComponents = inputs.size();
			for (size_t i = 0; i < indices.Count; i += vertexComponents)
			{
				//Construct vertices
                    int32_t pos = indices.pData[i + positionOffset];
                    vertex.Position = (positionOffset >= 0) ? mesh.pPositions[pos] : glm::vec3();

                    int32_t norm = indices.pData[i + normalOffset];
                    vertex.Normal = (normalOffset >= 0) ? mesh.pNormals[norm] : glm::vec3();

                    int32_t uv = indices.pData[i + texcoordOffset];
                    vertex.TexCoords = (texcoordOffset >= 0) ? mesh.pTexCoords[uv] : glm::vec3();
            
				InsertUniqueVertex(mesh, vertex);
			}

			//Go to the next list
			pTriangles = pTriangles->NextSiblingElement("triangles");
		}
    }
    else
    {
#if DEBUG_PRINTS
        ThreadSafePrintf("COLLADA-Mesh does not contain any triangles-element\n");
#endif
    }
}


inline void ReadPolylists(COLLADAMesh& mesh, std::unordered_map<std::string, COLLADASource>& sources, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    //Get triangles
    XMLElement* pPolylist = pParent->FirstChildElement("polylist");
	if (pPolylist != nullptr)
	{
		while (pPolylist != nullptr)
		{
			//Get input
			std::unordered_map<std::string, COLLADAInput> inputs;
			ReadInputs(inputs, pPolylist);
        
			//Construct normals
			ConstructVec3("NORMAL", &mesh.pNormals, mesh.NumNormals, sources, inputs);
			//Construct texcoords
			ConstructVec2("TEXCOORD", &mesh.pTexCoords, mesh.NumTexCoords, sources, inputs);

			//Read normal components
			int32_t normalOffset = -1;
			{
				auto normal = inputs.find("NORMAL");
				if (normal != inputs.end())
					normalOffset = normal->second.Offset;
			}       
			//Read texcoords
			int32_t texcoordOffset = -1;
			{
				auto texcoord = inputs.find("TEXCOORD");
				if (texcoord != inputs.end())
					texcoordOffset = texcoord->second.Offset;
			}
			//Read position offset
			int32_t positionOffset = -1;
			{
				auto position = inputs.find("VERTEX");
				if (position != inputs.end())
					positionOffset = position->second.Offset;
			}
        
			//Get number of polygons
			int32_t polyCount = 0;
			XMLError result = pPolylist->QueryIntAttribute("count", &polyCount);
			assert(result == XML_SUCCESS);
        
			//Get the vcount - This is the number of vertices per triangle
			COLLADAIntArray vcount = ReadVCount(polyCount, pPolylist);
			if (vcount.pData == nullptr)
			{
				ThreadSafePrintf("COLLADA-Mesh does not contain any vcount\n");
				return;
			}
        
			//Count the total amount of vertices
			size_t vertexCount = 0;
            for (int32_t i = 0; i < vcount.Count; i++)
            {
				vertexCount += vcount.pData[i];
            }
        
			//Get the indices (Count of indices is 'elementsPerVertex * vertexCount')
			COLLADAIntArray indices = ReadIndices(uint32_t(inputs.size()) * uint32_t(vertexCount), pPolylist);
			if (indices.pData == nullptr)
			{
				ThreadSafePrintf("COLLADA file does not contain any indices\n");
				return;
			}
        
			//Process faces
			Vertex vertex = {};
			int32_t baseIndex = 0;
			int32_t indicesPerTriangle = 3 * int32_t(inputs.size());
            for (int32_t i = 0; i < vcount.Count; i++)
			{
                int32_t& vc = vcount.pData[i];
				assert(vc >= 3);

				//Read in three vertices
				for (int32_t j = 0; j < indicesPerTriangle; j += int32_t(inputs.size()))
				{
					//Construct vertices
                    int32_t pos = indices.pData[baseIndex + j + positionOffset];
					vertex.Position  = (positionOffset >= 0)  ? mesh.pPositions[pos] : glm::vec3();

                    int32_t norm = indices.pData[baseIndex + j + normalOffset];
					vertex.Normal    = (normalOffset >= 0)    ? mesh.pNormals[norm] : glm::vec3();

                    int32_t uv = indices.pData[baseIndex + j + texcoordOffset];
					vertex.TexCoords = (texcoordOffset >= 0)  ? mesh.pTexCoords[uv] : glm::vec3();
                
					InsertUniqueVertex(mesh, vertex);
				}
            
				//Calculate the indices for this polygon
				int32_t indicesPerPolygon = vc * int32_t(inputs.size());
            
				//Check if we have a polygon and convert it to triangles
				size_t numIndices   = mesh.Indices.size();
				uint32_t startIndex = mesh.Indices[numIndices - 3]; //Get the first vertex in the base triangle
				for (int32_t j = indicesPerTriangle; j < indicesPerPolygon; j += int32_t(inputs.size()))
				{
                    //Construct vertices
                    int32_t pos = indices.pData[baseIndex + j + positionOffset];
                    vertex.Position = (positionOffset >= 0) ? mesh.pPositions[pos] : glm::vec3();

                    int32_t norm = indices.pData[baseIndex + j + normalOffset];
                    vertex.Normal = (normalOffset >= 0) ? mesh.pNormals[norm] : glm::vec3();

                    int32_t uv = indices.pData[baseIndex + j + texcoordOffset];
                    vertex.TexCoords = (texcoordOffset >= 0) ? mesh.pTexCoords[uv] : glm::vec3();
                
					//Add a new triangle base on the last couple of ones
					mesh.Indices.emplace_back(startIndex);
					mesh.Indices.emplace_back(mesh.Indices[numIndices - 1]);

					InsertUniqueVertex(mesh, vertex);
				}

				//Add to baseindex
				baseIndex += indicesPerPolygon;
			}

			//Go to the next list
			pPolylist = pPolylist->NextSiblingElement("polylist");
		}
	}
    else
    {
#if DEBUG_PRINTS
        ThreadSafePrintf("COLLADA-Mesh does not contain any polylist\n");
#endif
    }
}


inline void GetParams(COLLADAAccessor& accessor, tinyxml2::XMLElement* pAccessor)
{
    using namespace tinyxml2;
    
    COLLADAParam param = {};

    //Get technique node
    XMLElement* pParam = pAccessor->FirstChildElement("param");
    while (pParam != nullptr)
    {
        //Get param name
        const char* name = pParam->Attribute("name");
        if (name != nullptr)
            strcpy(param.Name, name);
        //Get param type
        const char* type = pParam->Attribute("type");
        if (type != nullptr)
            strcpy(param.Type, type);
        
        accessor.Params.push_back(param);
        pParam = pParam->NextSiblingElement("param");
    }
}


inline COLLADAAccessor GetAccessor(tinyxml2::XMLElement* pSource)
{
    using namespace tinyxml2;
    
    COLLADAAccessor accessor = {};

    //Get technique node
    XMLElement* pTechnique = pSource->FirstChildElement("technique_common");
    if (pTechnique == nullptr)
    {
        return accessor;
    }
    
    //Get technique node
    XMLElement* pAccessor = pTechnique->FirstChildElement("accessor");
    if (pAccessor != nullptr)
    {
        //Get source id
        const char* source = pAccessor->Attribute("source");
        assert(source != nullptr);
        strcpy(accessor.Source, source);
        
        //Get count
        XMLError result = pAccessor->QueryIntAttribute("count", &accessor.Count);
        assert(result == XML_SUCCESS);
        //Get stride
        result = pAccessor->QueryIntAttribute("stride", &accessor.Stride);
        assert(result == XML_SUCCESS);
        
        GetParams(accessor, pAccessor);
    }
    
    return accessor;
}


inline void GetFloatArray(COLLADAFloatArray& array, tinyxml2::XMLElement* pSource)
{
    using namespace tinyxml2;

    //Get technique node
    XMLElement* pArray = pSource->FirstChildElement("float_array");
    if (pArray != nullptr)
    {
        //Get ID of array
        const char* ID = pArray->Attribute("id");
        if (ID != nullptr)
            strcpy(array.ID, ID);
        
        //Get count
        XMLError result = pArray->QueryIntAttribute("count", &array.Count);
        assert(result == XML_SUCCESS);
        
        //Get data
        array.pData = (float*)stack_allocate(sizeof(float) * array.Count, 1, "Vertices");
        memset(array.pData, 0, sizeof(int32_t) * array.Count);
        const char* pText = pArray->GetText();

        //Parse the float data
        int32_t length = 0;
        const char* pIter = pText;
        for (int32_t i = 0; i < array.Count; i++)
        {
            array.pData[i] = float(FastAtof(pIter, length));
            pIter += length;
            if (*pIter == ' ')
                pIter++;
        }
    }
}


inline void ReadSources(std::unordered_map<std::string, COLLADASource>& sources, tinyxml2::XMLElement* pMesh)
{
    using namespace tinyxml2;

#if DEBUG_PRINTS
    size_t oldSize = sources.size();
#endif
	COLLADASource source = {};

    XMLElement* pSource = pMesh->FirstChildElement("source");
    while (pSource != nullptr)
    { 
        //Get attributes
        const char* ID = pSource->Attribute("id");
        assert(ID != nullptr);
        strcpy(source.ID, ID);
        
        //Get techniques
        source.Accessor   = GetAccessor(pSource);
        GetFloatArray(source.FloatArray, pSource);
        
        //Emplace current source and move to the next one
        sources.emplace(std::make_pair(source.ID, source));
        pSource = pSource->NextSiblingElement("source");

		//Clear previous source
		source.FloatArray.pData = nullptr;
		source.FloatArray.Count = 0;
		memset(source.ID, 0, sizeof(source.ID));
    }
    
#if DEBUG_PRINTS
    ThreadSafePrintf("COLLADA-Mesh contains %d sources\n", sources.size() - oldSize);
#endif
}


inline bool ReadMeshes(std::vector<MeshData>& meshes, tinyxml2::XMLElement* pRoot)
{
    using namespace tinyxml2;
    
    //Get library_geometries
    XMLElement* pLibrary = pRoot->FirstChildElement("library_geometries");
    if (pLibrary == nullptr)
    {
        ThreadSafePrintf("COLLADA file does not contain elemement 'library_geometries'\n");
        return false;
    }
    
    //All the sources for a mesh
    std::unordered_map<std::string, COLLADASource> sources;
	MeshData mesh = {};
	COLLADAMesh colladaMesh = {};

	//Read all geometry
    XMLElement* pGeometry = pLibrary->FirstChildElement("geometry");
    while (pGeometry != nullptr)
    {
        XMLElement* pMesh = pGeometry->FirstChildElement("mesh");
        while (pMesh != nullptr)
        {
            ReadSources(sources, pMesh);
            ReadVertices(colladaMesh, sources, pMesh);
            ReadTriangles(colladaMesh, sources, pMesh);
            ReadPolylists(colladaMesh, sources, pMesh);
             
			//Take all the polylists and triangles and put them together into a single mesh
			mesh.Indices.swap(colladaMesh.Indices);
			mesh.Vertices.swap(colladaMesh.Vertices);

            //Go to next mesh element
			meshes.emplace_back(mesh);
            pMesh = pMesh->NextSiblingElement("mesh");

			//Clear data from last mesh
			mesh.Indices.clear();
			mesh.Vertices.clear();
			colladaMesh.Indices.clear();
			colladaMesh.pNormals = nullptr;
            colladaMesh.NumNormals = 0;
            colladaMesh.pPositions = nullptr;
            colladaMesh.NumPositions = 0;
			colladaMesh.pTexCoords = nullptr;
            colladaMesh.NumTexCoords = 0;
			colladaMesh.UniqueVertices.clear();
			colladaMesh.Vertices.clear();
        }
        
        //Go to mext geometry element
        pGeometry = pGeometry->NextSiblingElement("geometry");
    }
    
    //Write warning
    if (meshes.size() > 1)
        ThreadSafePrintf("WARNING: COLLADA file contains %d meshes, will be merged to 1\n", meshes.size());
    
    return true;
}


MeshData LoaderCOLLADA::ReadFromDisk(const std::string& filepath)
{
    using namespace tinyxml2;
    
    stack_reset();

    //Read in document
    XMLDocument doc = {};
    XMLError result = doc.LoadFile(filepath.c_str());
    if (result != XML_SUCCESS)
    {
        ThreadSafePrintf("Failed to load file '%s'\n", filepath.c_str());
        return MeshData();
    }
    else
    {
        ThreadSafePrintf("Loaded file '%s'\n", filepath.c_str());
    }
    
    //Get the first node
    XMLElement* pFirst = doc.FirstChildElement();
    if (pFirst == nullptr)
    {
        ThreadSafePrintf("File '%s' is empty\n", filepath.c_str());
        return MeshData();
    }
    
    
    //Convert name to lower
    char buff[512];
    memset(buff, 0, sizeof(buff));
    const char* pName = pFirst->Name();
    for (int32_t i = 0; pName[i] != 0; i++)
        buff[i] = (char)tolower(int(pName[i]));
    
    
    //Check so that the first element is a COLLADA element
    if (strcmp("collada", buff) != 0)
    {
        ThreadSafePrintf("File does not seem to be a collada file since the first element is not <COLLADA>\n");
        return MeshData();
    }
    
    
    //Get all the meshes in the COLLADA scene
	std::vector<MeshData> meshes;
	if (!ReadMeshes(meshes, pFirst))
	{
		ThreadSafePrintf("Failed to read meshes\n");
		return MeshData();
	}

    if (meshes.size() > 1)
    {
        //If we have more than one mesh, we combine them into a single mesh
        MeshData singleMesh = {};
        uint32_t vertexOffset = 0;
        for (auto& mesh : meshes)
        {
            //Add all vertices for this mesh
            for (auto& vertex : mesh.Vertices)
                singleMesh.Vertices.emplace_back(vertex);    
            //Add all indices for this mesh
            for (auto& index : mesh.Indices)
                singleMesh.Indices.emplace_back(index + vertexOffset); //Add the vertexoffset to the index
            //Set offset
            vertexOffset = uint32_t(singleMesh.Vertices.size());
        }
        
        //Print number of vertices, indices and triangles
        ThreadSafePrintf("Finished loading COLLADA-file '%s' - VertexCount=%d, IndexCount=%d, TriangleCount=%d\n", filepath.c_str(), singleMesh.Vertices.size(), singleMesh.Indices.size(), singleMesh.Indices.size() / 3);
        
        stack_reset();
        return singleMesh;
    }
    else
    {
        //Print number of vertices, indices and triangles
        ThreadSafePrintf("Finished loading COLLADA-file '%s' - VertexCount=%d, IndexCount=%d, TriangleCount=%d\n", filepath.c_str(), meshes[0].Vertices.size(), meshes[0].Indices.size(), meshes[0].Indices.size() / 3);
        
        stack_reset();
        return meshes.front();
    }
}
