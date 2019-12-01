#pragma once
#include "Helpers.h"
#include "PoolAllocator.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 texCoords;

public:
	Vertex() {};
	Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec3& t, const glm::vec2& tc)
	{
		position = p;
		normal = n;
		tangent = t;
		texCoords = tc;
	};

	inline bool operator==(const Vertex& rs) const
	{
		return (position == rs.position) && (normal == rs.normal) && (tangent == rs.tangent) && (texCoords == rs.texCoords);
	};
};


class Mesh
{
public:
	Mesh(Mesh&& other) = delete;
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(Mesh&& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;

	Mesh(const Vertex* const vertices, const uint32_t* const indices, uint32_t numVertices, uint32_t numIndices);
	~Mesh();

	void Construct();
	void Draw();

	inline uint32_t GetIndexCount() const noexcept
	{
		return m_IndexCount;
	}

	inline uint32_t GetVertexCount() const noexcept
	{
		return m_VertexCount;
	}

	inline void* operator new(size_t, const char* tag)
	{
#ifdef SHOW_ALLOCATIONS_DEBUG
		return PoolAllocator<Mesh>::Get().AllocateBlock(tag);
#else
		return PoolAllocator<Mesh>::Get().AllocateBlock();
#endif
	}

	inline void operator delete(void* ptr)
	{
		PoolAllocator<Mesh>::Get().Free((Mesh*)ptr);
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

