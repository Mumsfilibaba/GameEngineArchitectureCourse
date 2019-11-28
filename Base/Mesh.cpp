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
	m_VertexCount = numVertices;
	m_IndexCount = numIndices;
	m_pVertices = vertices;
	m_pIndices = indices;
}

Mesh::~Mesh()
{
	if (glIsVertexArray(m_VAO))
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
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

void Mesh::Construct()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_VertexCount * sizeof(Vertex), m_pVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(uint32_t), m_pIndices, GL_STATIC_DRAW);

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

	delete m_pVertices;
	release((void*)m_pIndices);
}

void Mesh::Draw()
{
	glBindVertexArray(this->m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


Mesh* Mesh::CreateCube()
{
	Vertex* triangleVertices = new Vertex[24]
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

	void* pMem = allocate(sizeof(uint32_t) * 36, 1, "Cube Indices");
	uint32_t* triangleIndices = (uint32_t*)pMem;
	
	// Front (Seen from front)
	triangleIndices[0] = 0;
	triangleIndices[1] = 2;
	triangleIndices[2] = 1;
	triangleIndices[3] = 2;
	triangleIndices[4] = 0;
	triangleIndices[5] = 3;

	// Top (Seen from above)
	triangleIndices[6] = 4;
	triangleIndices[7] = 6;
	triangleIndices[8] = 5;
	triangleIndices[9] = 6;
	triangleIndices[10] = 4;
	triangleIndices[11] = 7;

	// Back (Seen from front)
	triangleIndices[12] = 8;
	triangleIndices[13] = 9;
	triangleIndices[14] = 10;
	triangleIndices[15] = 10;
	triangleIndices[16] = 11;
	triangleIndices[17] = 8;

	// Bottom (Seen from above)
	triangleIndices[18] = 12;
	triangleIndices[19] = 13;
	triangleIndices[20] = 14;
	triangleIndices[21] = 14;
	triangleIndices[22] = 15;
	triangleIndices[23] = 12;

	// Left (Seen from left)
	triangleIndices[24] = 16;
	triangleIndices[25] = 18;
	triangleIndices[26] = 18;
	triangleIndices[27] = 18;
	triangleIndices[28] = 16;
	triangleIndices[29] = 19;

	// Right (Seen from left)
	triangleIndices[30] = 20;
	triangleIndices[31] = 21;
	triangleIndices[32] = 22;
	triangleIndices[33] = 22;
	triangleIndices[34] = 23;
	triangleIndices[35] = 20;

	return new("Cube Mesh") Mesh(triangleVertices, triangleIndices, 24, 36);
}

Mesh* Mesh::CreateCubeInvNormals()
{
	Vertex* triangleVertices = new Vertex[24]
	{
		// Front (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Top (Seen from above)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(0.0F,  1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Back (Seen from front)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F,  0.0F, -1.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Bottom (Seen from above)
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(0.0F, -1.0F,  0.0F),	 glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec2(0.0F, 0.0F)),

		// Left (Seen from left)
		Vertex(glm::vec3(-0.5F,  0.5F, -0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F,  0.5F,  0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F, -0.5F),	-glm::vec3(-1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F)),

		// Right (Seen from left)
		Vertex(glm::vec3(0.5F,  0.5F, -0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(0.5F, -0.5F, -0.5F),	-glm::vec3(1.0F,  0.0F,  0.0F),	 glm::vec3(0.0F,  0.0F,  1.0F),	 glm::vec2(0.0F, 0.0F))
	};

	uint32_t* triangleIndices = new uint32_t[36]
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
	Vertex* triangleVertices = new Vertex[4]
	{
		Vertex(glm::vec3(-0.5F,  0.5F,  0.0F), glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(0.0F, 1.0F)),
		Vertex(glm::vec3(0.5F,  0.5F,  0.0F),  glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(1.0F, 1.0F)),
		Vertex(glm::vec3(0.5F, -0.5F,  0.0F),  glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(1.0F, 0.0F)),
		Vertex(glm::vec3(-0.5F, -0.5F,  0.0F), glm::vec3(0.0F,  0.0F,  1.0F), glm::vec3(1.0F,  0.0F,  0.0F), glm::vec2(0.0F, 0.0F))
	};

	uint32_t* triangleIndices = new uint32_t[6]
	{
		// Front (Seen from front)
		0, 2, 1,
		2, 0, 3
	};

	return new("Tri Mesh")  Mesh(triangleVertices, triangleIndices, 4, 6);
}