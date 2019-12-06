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

void Mesh::Init()
{
	GL_CALL(glGenBuffers(1, &m_VBO));
	GL_CALL(glGenBuffers(1, &m_IBO));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_VertexCount * sizeof(Vertex), m_pVertices, GL_STATIC_DRAW));

	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(uint32_t), m_pIndices, GL_STATIC_DRAW));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	mm_free((void*)m_pVertices);
	m_pVertices = nullptr;

	mm_free((void*)m_pIndices);
	m_pIndices = nullptr;
}

void Mesh::Release()
{
	
}

void Mesh::Draw(const sf::Shader& shader)
{
	//Bind buffers
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO));
    
    //Get the native handle
    GLuint program = shader.getNativeHandle();
    
    //Position
    GLint vertexLoc = glGetAttribLocation(program, "a_Position");
	if (vertexLoc >= 0)
	{
		GL_CALL(glEnableVertexAttribArray(vertexLoc));
		GL_CALL(glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
	}
    
	//Normal
    GLint normalLoc = glGetAttribLocation(program, "a_Normal");
	if (normalLoc >= 0)
	{
		GL_CALL(glEnableVertexAttribArray(normalLoc));
		GL_CALL(glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float))));
	}

    //TexCoords
    GLint texCoordLoc = glGetAttribLocation(program, "a_TexCoord");
	if (texCoordLoc >= 0)
	{
		GL_CALL(glEnableVertexAttribArray(texCoordLoc));
		GL_CALL(glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float))));
	}
    
    //Draw
	GL_CALL(glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr));

	//Unbind buffers
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    
    //We must reset the GL-States that we use since it otherwise interfere with SFML
    GL_CALL(glDisableVertexAttribArray(vertexLoc));
    GL_CALL(glDisableVertexAttribArray(normalLoc));
    GL_CALL(glDisableVertexAttribArray(texCoordLoc));
}


Mesh* Mesh::CreateCube()
{
	Vertex* triangleVertices = new(mm_allocate(sizeof(Vertex) * 24, 1, "Cube Vertices")) Vertex[24]
	{
		// Front (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	glm::vec3(0.0F,  0.0F,  1.0F),	glm::vec2(0.0F, 0.0F)),

		// Top (Seen from above)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	glm::vec3(0.0F,  1.0F,  0.0F),	glm::vec2(0.0F, 0.0F)),

		// Back (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	glm::vec3(0.0F,  0.0F, -1.0F),	glm::vec2(0.0F, 0.0F)),

		// Bottom (Seen from above)
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	glm::vec3(0.0F, -1.0F,  0.0F),	glm::vec2(0.0F, 0.0F)),

		// Left (Seen from left)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	glm::vec3(-1.0F,  0.0F,  0.0F),	glm::vec2(0.0F, 0.0F)),

		// Right (Seen from left)
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	glm::vec3(1.0F,  0.0F,  0.0F),	glm::vec2(0.0F, 0.0F))
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
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F)),
    
		// Top (Seen from above)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),
    
		// Back (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec2(0.0F, 0.0F)),
    
		// Bottom (Seen from above)
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Left (Seen from left)
        Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
        Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
        Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
        Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Right (Seen from left)
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F))
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
		Vertex(glm::vec3(-0.5F,  0.5F,  0.0F), glm::vec3(0.0F,  0.0F,  1.0F),  glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.0F),  glm::vec3(0.0F,  0.0F,  1.0F),  glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.0F),  glm::vec3(0.0F,  0.0F,  1.0F),  glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.0F), glm::vec3(0.0F,  0.0F,  1.0F),  glm::vec2(0.0F, 0.0F))
	};

	uint32_t* quadIndices = new(mm_allocate(sizeof(uint32_t) * 6, 1, "Quad Indices")) uint32_t[6]
	{
		// Front (Seen from front)
		0, 2, 1,
		2, 0, 3
	};

	return new("Quad Mesh") Mesh(quadVertices, quadIndices, 4, 6);
}
