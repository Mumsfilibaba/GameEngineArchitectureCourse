#include "LoaderCOLLADA.h"
#include <tinyxml2.h>
#include <string>
#include <unordered_map>

#define DEBUG_PRINTS 1

IResource* LoaderCOLLADA::LoadFromDisk(const std::string& file)
{
    std::vector<MeshData> meshes = std::move(ReadFromDisk(file));
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


IResource* LoaderCOLLADA::LoadFromMemory(void* pData, size_t)
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


size_t LoaderCOLLADA::WriteToBuffer(const std::string& file, void* buffer)
{
    std::vector<MeshData> meshes = std::move(ReadFromDisk(file));
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



template<typename T>
struct TArray
{
	std::string ID;
    std::vector<T> Data;
    int32_t Count;
};
using COLLADAFloatArray = TArray<float>;
using COLLADAIntArray = TArray<int32_t>;


struct COLLADAInput
{
    std::string Semantic;
    std::string Source;
    int32_t Offset = -1;
};


struct COLLADAParam
{
    std::string Name;
    std::string Type;
};


struct COLLADAAccessor
{
    std::string Source;
    int32_t Count;
    int32_t Stride;
    std::vector<COLLADAParam> Params;
};


struct COLLADASource
{
    std::string ID;
    COLLADAFloatArray FloatArray;
    COLLADAAccessor Accessor;
};


struct COLLADAMesh
{
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> TexCoords;
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    std::unordered_map<Vertex, uint32_t> UniqueVertices;
};


inline COLLADAIntArray GetIndices(uint32_t count, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    COLLADAIntArray indices = {};
    XMLElement* pIndices = pParent->FirstChildElement("p");
    if (pIndices != nullptr)
    {
        indices.Count = count;
        indices.Data.resize(count);
        const char* pText = pIndices->GetText();
        
        //Parse the float data
        int32_t length = 0;
        const char* pIter = pText;
        for (int32_t i = 0; i < indices.Count; i++)
        {
            indices.Data[i] = FastAtof(pIter, length);
            pIter += length;
            if (*pIter == ' ')
                pIter++;
        }
    }
    
    return indices;
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
        input.Semantic = semantic;
        
        //Get source
        const char* source = pInput->Attribute("source");
        assert(semantic != nullptr);
        input.Source = source;
        
        //Get offset and set
        XMLError result = pInput->QueryIntAttribute("offset", &input.Offset);
        if (result != XML_SUCCESS)
            input.Offset = -1;
        
        //Go to next
        inputs.emplace(std::make_pair(input.Semantic, input));
        pInput = pInput->NextSiblingElement("input");
    }
}


inline void ConstructVec3(const std::string& semantic, std::vector<glm::vec3>& vec, std::unordered_map<std::string, COLLADASource>& sources, std::unordered_map<std::string, COLLADAInput>& inputs)
{
    //Read positions and construct positions
    auto input = inputs.find(semantic);
    if (input != inputs.end())
    {
        //Get source ID, but remove the # in the beginning
        std::string sourceID = input->second.Source.substr(1);
        
        //Get the source and construct positions
        auto source = sources.find(sourceID);
        if (source != sources.end())
        {
            uint32_t componentCount = source->second.Accessor.Params.size();
            assert(componentCount == 3); // We only support vec3 here
            
            auto& data = source->second.FloatArray.Data;
            for (size_t i = 0; i < data.size(); i += 3)
                vec.emplace_back(data[i], data[i+1], data[i+2]);
        }
        else
        {
            ThreadSafePrintf("Source '%s' in COLLADA file was not found\n", input->second.Source.c_str());
        }
    }
}


inline void ConstructVec2(const std::string& semantic, std::vector<glm::vec2>& vec, std::unordered_map<std::string, COLLADASource>& sources, std::unordered_map<std::string, COLLADAInput>& inputs)
{
    //Read positions and construct positions
    auto input = inputs.find(semantic);
    if (input != inputs.end())
    {
        //Get source ID, but remove the # in the beginning
        std::string sourceID = input->second.Source.substr(1);
        
        //Get the source and construct positions
        auto source = sources.find(sourceID);
        if (source != sources.end())
        {
            uint32_t componentCount = source->second.Accessor.Params.size();
            assert(componentCount == 2); // We only support vec2 here
            
            auto& data = source->second.FloatArray.Data;
            for (size_t i = 0; i < data.size(); i += 2)
                vec.emplace_back(data[i], data[i+1]);
        }
        else
        {
            ThreadSafePrintf("Source '%s' in COLLADA file was not found\n", input->second.Source.c_str());
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
        ConstructVec3("POSITION", mesh.Positions, sources, inputs);
    }
    else
    {
        ThreadSafePrintf("COLLADA file does not contain any vertices\n");
    }
}


inline void ReadTriangles(COLLADAMesh& mesh, std::unordered_map<std::string, COLLADASource>& sources, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    //Get triangles
    XMLElement* pTriangles = pParent->FirstChildElement("triangles");
    if (pTriangles == nullptr)
    {
        //Get polylist element if no triangles element exists
        pTriangles = pParent->FirstChildElement("polylist");
    }
    
    //Do the actual parsing
    if (pTriangles != nullptr)
    {
        //Get input
        std::unordered_map<std::string, COLLADAInput> inputs;
        ReadInputs(inputs, pTriangles);
        
        //Get trianglecount
        int32_t triangleCount = 0;
        XMLError result = pTriangles->QueryIntAttribute("count", &triangleCount);
        assert(result == XML_SUCCESS);
        
        //Get the indices
        COLLADAIntArray indices = GetIndices(inputs.size() * triangleCount * 3, pTriangles);
        if (indices.Data.empty())
        {
            ThreadSafePrintf("COLLADA file does not contain any indices\n");
            return;
        }
        
        //Construct normals
        ConstructVec3("NORMAL", mesh.Normals, sources, inputs);
        //Construct texcoords
        ConstructVec2("TEXCOORD", mesh.TexCoords, sources, inputs);

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
        for (size_t i = 0; i < indices.Data.size(); i += vertexComponents)
        {
            //Construct vertices
            vertex.Position  = (positionOffset >= 0)  ? mesh.Positions[indices.Data[i + positionOffset]]    : glm::vec3();
            vertex.Normal    = (normalOffset >= 0)    ? mesh.Normals[indices.Data[i + normalOffset]]        : glm::vec3();
            vertex.TexCoords = (texcoordOffset >= 0)  ? mesh.TexCoords[indices.Data[i + texcoordOffset]]    : glm::vec3();
            
            //Insert unique vertex
            if (mesh.UniqueVertices.count(vertex) == 0)
            {
                mesh.UniqueVertices[vertex] = (uint32_t)mesh.Vertices.size();
                mesh.Vertices.push_back(vertex);
            }

            mesh.Indices.emplace_back(mesh.UniqueVertices[vertex]);
        }
    }
    else
    {
        ThreadSafePrintf("COLLADA file does not contain any triangles or polylist\n");
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
            param.Name = name;
        //Get param type
        const char* type = pParam->Attribute("type");
        if (type != nullptr)
            param.Type = type;
        
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
        accessor.Source = source;
        
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


inline COLLADAFloatArray GetFloatArray(tinyxml2::XMLElement* pSource)
{
    using namespace tinyxml2;
    
    COLLADAFloatArray array = {};

    //Get technique node
    XMLElement* pArray = pSource->FirstChildElement("float_array");
    if (pArray != nullptr)
    {
        //Get ID of array
        const char* ID = pArray->Attribute("id");
        if (ID != nullptr)
            array.ID = ID;
        
        //Get count
        XMLError result = pArray->QueryIntAttribute("count", &array.Count);
        assert(result == XML_SUCCESS);
        
        //Get data
        array.Data.resize(array.Count);
        const char* pText = pArray->GetText();

        //Parse the float data
        int32_t length = 0;
        const char* pIter = pText;
        for (int32_t i = 0; i < array.Count; i++)
        {
            array.Data[i] = FastAtof(pIter, length);
            pIter += length;
            if (*pIter == ' ')
                pIter++;
        }
    }
    
    return array;
}


inline void ReadSources(std::unordered_map<std::string, COLLADASource>& sources, tinyxml2::XMLElement* pMesh)
{
    using namespace tinyxml2;
    
    XMLElement* pSource = pMesh->FirstChildElement("source");
    while (pSource != nullptr)
    {
        COLLADASource source = {};
        
        //Get attributes
        const char* ID = pSource->Attribute("id");
        assert(ID != nullptr);
        source.ID = ID;
        
        //Get techniques
        source.Accessor   = GetAccessor(pSource);
        source.FloatArray = GetFloatArray(pSource);
        
        //Emplace current source and move to the next one
        sources.emplace(std::make_pair(source.ID, source));
        pSource = pSource->NextSiblingElement("source");
    }
    
    ThreadSafePrintf("COLLADA file contains %d sources\n", sources.size());
}


inline std::vector<MeshData> ReadMeshes(tinyxml2::XMLElement* pRoot)
{
    using namespace tinyxml2;
    
    std::vector<MeshData> meshes;
    
    //Get library_geometries
    XMLElement* pLibrary = pRoot->FirstChildElement("library_geometries");
    if (pLibrary == nullptr)
    {
        ThreadSafePrintf("COLLADA file does not contain elemement 'library_geometries'\n");
        return std::vector<MeshData>();
    }
    
    //All the sources for a mesh
    std::unordered_map<std::string, COLLADASource> sources;
    
    //Read all geometry
    XMLElement* pGeometry = pLibrary->FirstChildElement("geometry");
    while (pGeometry != nullptr)
    {
        XMLElement* pMesh = pGeometry->FirstChildElement("mesh");
        while (pMesh != nullptr)
        {
            COLLADAMesh mesh = {};
            ReadSources(sources, pMesh);
            ReadVertices(mesh, sources, pMesh);
            ReadTriangles(mesh, sources, pMesh);
           
            //Set mesh
            MeshData meshData = {};
            meshData.Vertices.swap(mesh.Vertices);
            meshData.Indices.swap(mesh.Indices);
            meshes.emplace_back(meshData);
            
            //Go to next mesh element
            pMesh = pMesh->NextSiblingElement("mesh");
        }
        
        //Go to mext geometry element
        pGeometry = pGeometry->NextSiblingElement("geometry");
    }
    
    return meshes;
}


std::vector<MeshData> LoaderCOLLADA::ReadFromDisk(const std::string& filepath)
{
    using namespace tinyxml2;
    
    //Read in document
    XMLDocument doc = {};
    XMLError result = doc.LoadFile(filepath.c_str());
    if (result != XML_SUCCESS)
    {
        ThreadSafePrintf("Failed to load file '%s'\n", filepath.c_str());
        return std::vector<MeshData>();
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
        return std::vector<MeshData>();
    }
    
    //Get all the meshes in the COLLADA scene
    return ReadMeshes(pFirst);
}
