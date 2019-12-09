#include "LoaderCOLLADA.h"
#include <tinyxml2.h>

#define DEBUG_PRINTS 1

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
    std::vector<T> Data;
    int32_t Count;
};
using COLLADAFloatArray = TArray<float>;
using P = TArray<float>;

struct COLLADAInput
{
    std::string Semantic;
    std::string Source;
    int32_t Offset;
    int32_t Set;
};


struct COLLADAVertices
{
    std::string ID;
    std::vector<COLLADAInput> Inputs;
};


struct COLLADATriangles
{
    std::string Material;
    std::vector<COLLADAInput> Inputs;
    int32_t Count;
    P Indices;
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


struct COLLADATechnique
{
    COLLADAAccessor Accessor;
};


struct COLLADASource
{
    std::string ID;
    COLLADAFloatArray FloatArray;
    COLLADATechnique Technique;
};


struct COLLADAMesh
{
    std::vector<COLLADASource> Sources;
    std::vector<COLLADAVertices> Vertices;
    std::vector<COLLADATriangles> Triangles;
};


struct COLLADAGeometry
{
	std::string ID;
	std::string Name;
    std::vector<COLLADAMesh> Meshes;
};


struct COLLADALibraryGeometry
{
	std::vector<COLLADAGeometry> Geometries;
};


inline P GetP(COLLADATriangles& triangles, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    P p = {};
    XMLElement* pP = pParent->FirstChildElement("p");
    if (pP != nullptr)
    {
        //Get data (tricount * 3 * input.size) - number of triangles * vertices per triangle * inputs per vertex
        p.Data.resize(triangles.Count * 3 * triangles.Inputs.size());
        const char* pText = pP->GetText();
        
        //Parse the float data
        int32_t length = 0;
        const char* pIter = pText;
        for (int32_t i = 0; i < p.Count; i++)
        {
            p.Data[i] = FastAtof(pIter, length);
            pIter += length;
            if (*pIter == ' ')
                pIter++;
        }
    }
    
    return p;
}


inline void GetInputs(std::vector<COLLADAInput>& inputs, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    XMLElement* pInput = pParent->FirstChildElement("input");
    while (pInput != nullptr)
    {
        COLLADAInput input = {};
        
        //Get semantic
        const char* semantic = pInput->Attribute("semantic");
        if (semantic != nullptr)
            input.Semantic = semantic;
        //Get source
        const char* source = pInput->Attribute("source");
        if (source != nullptr)
            input.Source = source;
        //Get offset and set
        XMLError result = pInput->QueryIntAttribute("offset", &input.Offset);
        result = pInput->QueryIntAttribute("set", &input.Set);
        
        //Go to next
        inputs.push_back(input);
        pInput = pInput->NextSiblingElement("input");
    }
}


inline void GetVertices(std::vector<COLLADAVertices>& vertices, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    XMLElement* pVertices = pParent->FirstChildElement("vertices");
    while (pVertices != nullptr)
    {
        COLLADAVertices verts = {};
        
        //Get ID
        const char* ID = pVertices->Attribute("id");
        if (ID != nullptr)
            verts.ID = ID;
        //Get source
        GetInputs(verts.Inputs, pVertices);
        
        //Go to next
        vertices.push_back(verts);
        pVertices = pVertices->NextSiblingElement("vertices");
    }
}


inline void GetTriangles(std::vector<COLLADATriangles>& triangles, tinyxml2::XMLElement* pParent)
{
    using namespace tinyxml2;
    
    XMLElement* pTriangles = pParent->FirstChildElement("triangles");
    while (pTriangles != nullptr)
    {
        COLLADATriangles tris = {};
        
        //Get ID
        const char* material = pTriangles->Attribute("material");
        if (material != nullptr)
            tris.Material = material;
        
        //Get tri count
        XMLError result = pTriangles->QueryIntAttribute("count", &tris.Count);
        assert(result == XML_SUCCESS);
        
        //Get source
        GetInputs(tris.Inputs, pTriangles);
        
        //Get the indices
        tris.Indices = GetP(tris, pTriangles);
        
        //Go to next
        triangles.push_back(tris);
        pTriangles = pTriangles->NextSiblingElement("triangles");
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


inline COLLADAAccessor GetAccessor(tinyxml2::XMLElement* pTechnique)
{
    using namespace tinyxml2;
    
    COLLADAAccessor accessor = {};

    //Get technique node
    XMLElement* pAccessor = pTechnique->FirstChildElement("accessor");
    if (pAccessor != nullptr)
    {
        //Get source id
        const char* Source = pAccessor->Attribute("source");
        if (Source != nullptr)
            accessor.Source = Source;
        
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


inline COLLADATechnique GetTechnique(tinyxml2::XMLElement* pSource)
{
    using namespace tinyxml2;
    
    COLLADATechnique technique = {};

    //Get technique node
    XMLElement* pTechnique = pSource->FirstChildElement("technique_common");
    if (pTechnique != nullptr)
        technique.Accessor = GetAccessor(pTechnique);
    
    return technique;
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


inline void GetSources(COLLADAMesh& meshes, tinyxml2::XMLElement* pMesh)
{
    using namespace tinyxml2;
    
    XMLElement* pSource = pMesh->FirstChildElement("source");
    while (pSource != nullptr)
    {
        COLLADASource source = {};
        
        //Get attributes
        const char* ID = pSource->Attribute("id");
        if (ID != nullptr)
            source.ID = ID;
        
        //Get techniques
        source.Technique    = GetTechnique(pSource);
        source.FloatArray   = GetFloatArray(pSource);
        
        meshes.Sources.push_back(source);
        pSource = pSource->NextSiblingElement("source");
    }
}


inline void GetMeshes(COLLADAGeometry& geometry, tinyxml2::XMLElement* pGeometry)
{
    using namespace tinyxml2;
    
    XMLElement* pMesh = pGeometry->FirstChildElement("mesh");
    while (pMesh != nullptr)
    {
        COLLADAMesh mesh = {};
        
        //Get all sources inside mesh
        GetSources(mesh, pMesh);
        //Get all vertices
        GetVertices(mesh.Vertices, pMesh);
        //Get all triangles
        GetTriangles(mesh.Triangles, pMesh);
        
        geometry.Meshes.push_back(mesh);
        pMesh = pMesh->NextSiblingElement("mesh");
    }
}


inline void GetGeometries(COLLADALibraryGeometry& libraryGeometries, tinyxml2::XMLElement* pLibrary)
{
    using namespace tinyxml2;
    
    XMLElement* pGeometry = pLibrary->FirstChildElement("geometry");
    while (pGeometry != nullptr)
    {
        COLLADAGeometry geometry = {};
        
        //Get attributes
        const char* ID = pGeometry->Attribute("id");
        if (ID != nullptr)
        {
            geometry.ID = ID;
        }
        const char* name = pGeometry->Attribute("name");
        if (name != nullptr)
        {
            geometry.Name = name;
        }
        
        //Find all meshes
        GetMeshes(geometry, pGeometry);
        
        //Add geometry
        libraryGeometries.Geometries.push_back(geometry);
        pGeometry = pGeometry->NextSiblingElement("geometry");
    }
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
    
    //Get library_geometries
    XMLElement* pLibrary = pFirst->FirstChildElement("library_geometries");
    if (pLibrary == nullptr)
    {
        ThreadSafePrintf("File '%s' does not contain any library_geometries\n", filepath.c_str());
        return std::vector<MeshData>();
    }
    
    //Get all geometry
    COLLADALibraryGeometry libraryGeometries = {};
    GetGeometries(libraryGeometries, pLibrary);
    
	return std::vector<MeshData>();
}
