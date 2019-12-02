#include "Mesh.h"
#include "MemoryManager.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4291)		//Disable: "no matching operator delete found; memory will not be freed if initialization throws an exception"-warning
#endif

Mesh::Mesh(const Vertex* const vertices, const uint32_t* const indices, uint32_t numVertices, uint32_t numIndices)
	: m_VAO(0),
	m_VBO(0),
	m_IBO(0),
	m_VertexCount(0),
	m_IndexCount(0),
	m_pVertices(nullptr),
	m_pIndices(nullptr)
{
	m_VertexCount	= numVertices;
	m_IndexCount	= numIndices;
	m_pVertices		= vertices;
	m_pIndices		= indices;
}

Mesh::~Mesh()
{
	if (m_pVertices)
	{
		mm_free((void*)m_pVertices);
		m_pVertices = nullptr;
	}

	if (m_pIndices)
	{
		mm_free((void*)m_pIndices);
		m_pIndices = nullptr;
	}

	if (glIsBuffer(m_VBO))
	{
		glDeleteBuffers(1, &m_VBO);
		m_VBO = 0;
	}

	if (glIsBuffer(m_IBO))
	{
		glDeleteBuffers(1, &m_IBO);
		m_IBO = 0;
	}
}

void Mesh::Release()
{
}

void Mesh::Construct()
{
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_VertexCount * sizeof(Vertex), m_pVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(uint32_t), m_pIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	mm_free((void*)m_pVertices);
	m_pVertices = nullptr;

	mm_free((void*)m_pIndices);
	m_pIndices = nullptr;
}

void Mesh::Draw()
{
	//Bind buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    
    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    //Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //Tangent
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    //TexCoords
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
	glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    
	//We must reset the GL-States that we use since it otherwise interfere with SFML
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	//Unbind buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Mesh* Mesh::CreateCube()
{
	Vertex* triangleVertices = new(mm_allocate(sizeof(Vertex) * 24, 1, "Cube Vertices")) Vertex[24]
	{
		// Front (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Top (Seen from above)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Back (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Bottom (Seen from above)
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Left (Seen from left)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F)),

		// Right (Seen from left)
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F))
	};

    uint32_t* triangleIndices = new(mm_allocate(sizeof(uint32_t) * 36, 1, "Cube Indices")) uint32_t[36]
    {
        // Front (Seen from front)
        0, 2, 1,
        2, 0, 3,

        // Top (Seen from above)
        4, 6, 5,
        6, 4, 7,

        // Back (Seen from front)
        8, 9, 10,
        10, 11, 8,

        // Bottom (Seen from above)
        12, 13, 14,
        14, 15, 12,

        // Left (Seen from left)
        16, 18, 17,
        18, 16, 19,

        // Right (Seen from left)
        20, 21, 22,
        22, 23, 20
    };

	return new("Cube Mesh") Mesh(triangleVertices, triangleIndices, 24, 36);
}

Mesh* Mesh::CreateCubeInvNormals()
{
	Vertex* triangleVertices = new(mm_allocate(sizeof(Vertex) * 24, 1, "Cube InvNorm Vertices")) Vertex[24]
	{
		// Front (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),
    
		// Top (Seen from above)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),
    
		// Back (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),
    
		// Bottom (Seen from above)
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	    glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Left (Seen from left)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F)),

		// Right (Seen from left)
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	    glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F))
	};

	uint32_t* triangleIndices = new(mm_allocate(sizeof(uint32_t) * 36, 1, "Cube InvNorm Indices")) uint32_t[36]
	{
		// Front (Seen from front)
		0, 2, 1,
		2, 0, 3,

		// Top (Seen from above)
		4, 6, 5,
		6, 4, 7,

		// Back (Seen from front)
		8, 9, 10,
		10, 11, 8,

		// Bottom (Seen from above)
		12, 13, 14,
		14, 15, 12,

		// Left (Seen from left)
		16, 18, 17,
		18, 16, 19,

		// Right (Seen from left)
		20, 21, 22,
		22, 23, 20
	};

	return new("Cube InvNorm Mesh") Mesh(triangleVertices, triangleIndices, 24, 36);
}

Mesh* Mesh::CreateQuad()
{
	Vertex* quadVertices = new(mm_allocate(sizeof(Vertex) * 4, 1, "Quad Vertices")) Vertex[4]
	{
		Vertex(glm::vec3(-0.5F,  0.5F,  0.0F), glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.0F),  glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.0F),  glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.0F), glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(0.0F, 0.0F))
	};

	uint32_t* quadIndices = new(mm_allocate(sizeof(uint32_t) * 6, 1, "Quad Indices")) uint32_t[6]
	{
		// Front (Seen from front)
		0, 2, 1,
		2, 0, 3
	};

	return new("Quad Mesh") Mesh(quadVertices, quadIndices, 4, 6);
}
