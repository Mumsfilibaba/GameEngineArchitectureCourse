#pragma once
#include "Helpers.h"
#include "IResource.h"
#include "MemoryManager.h"
#include <SFML/Graphics/Shader.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//Define vertex used by application
struct Vertex
{
public:
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
public:
	Vertex() {};
	Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& tc)
	{
		Position = p;
		Normal = n;
		TexCoords = tc;
	};

	inline bool operator==(const Vertex& rs) const
	{
		return (Position == rs.Position) && (Normal == rs.Normal) && (TexCoords == rs.TexCoords);
	};
};

//Create hashfunction for a vertex
namespace std
{
    template<>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.TexCoords) << 1);
        }
    };
}


//HELPERS FOR MESHLOADING
struct MeshData
{
	std::vector<Vertex>     Vertices;
	std::vector<uint32_t>   Indices;
};

struct BinaryMeshData
{
    uint32_t VertexCount    = 0;
    uint32_t IndexCount     = 0;
};


//Define mesh
class Mesh : public IResource
{
public:
	Mesh(Mesh&& other) = delete;
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(Mesh&& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;

	Mesh(const Vertex* const vertices, const uint32_t* const indices, uint32_t numVertices, uint32_t numIndices);
	~Mesh();

	virtual void Init() override;
	virtual void Release() override;

	void Draw(const sf::Shader& shader);

	inline uint32_t GetIndexCount() const noexcept
	{
		return m_IndexCount;
	}

	inline uint32_t GetVertexCount() const noexcept
	{
		return m_VertexCount;
	}
private:
	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_IBO;
	uint32_t m_VertexCount;
	uint32_t m_IndexCount;
	const Vertex* m_pVertices;
	const uint32_t* m_pIndices;
public:
	static Mesh* CreateCube();
	static Mesh* CreateCubeInvNormals();
	static Mesh* CreateQuad();
};

