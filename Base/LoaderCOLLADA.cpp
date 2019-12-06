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
	size_t Count;
	std::vector<T> Data;
};
using COLLADAFloatArray = TArray<float>;


struct COLLADAAccessor
{
    std::string Source;
    int32_t Count;
    int32_t Stride;
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
        source.Technique = GetTechnique(pSource);
        
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
    
    //Debug print
#if DEBUG_PRINTS
    for (auto geometry : libraryGeometries.Geometries)
        ThreadSafePrintf("geometry id=\"%s\" name=\"%s\"\n", geometry.ID.c_str(), geometry.Name.c_str());
#endif
	return std::vector<MeshData>();
}
