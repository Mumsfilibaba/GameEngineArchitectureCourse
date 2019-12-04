#pragma once
#include "Helpers.h"
#include "IResource.h"
#include "MemoryManager.h"
#include <SFML/Graphics/Shader.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex
{
public:
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 TexCoords;
public:
	Vertex() {};
	Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec3& t, const glm::vec2& tc)
	{
		Position = p;
		Normal = n;
		Tangent = t;
		TexCoords = tc;
	};

	inline bool operator==(const Vertex& rs) const
	{
		return (Position == rs.Position) && (Normal == rs.Normal) && (Tangent == rs.Tangent) && (TexCoords == rs.TexCoords);
	};
};


//HELPER FOR MESHLOADING
struct MeshData
{
	std::vector<Vertex>  Vertices;
	std::vector<uint32_t>    Indices;
};


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

	void Construct();
	void Draw(const sf::Shader& shader);

	inline uint32_t GetIndexCount() const noexcept
	{
		return m_IndexCount;
	}

	inline uint32_t GetVertexCount() const noexcept
	{
		return m_VertexCount;
	}

	inline void* operator new(size_t size, const char* tag)
	{
#ifdef SHOW_ALLOCATIONS_DEBUG
		return mm_allocate(size, 1, tag);
#else
		return mm_allocate(size, 1, tag);
#endif
	}

	inline void operator delete(void* ptr)
	{
		mm_free(ptr);
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

