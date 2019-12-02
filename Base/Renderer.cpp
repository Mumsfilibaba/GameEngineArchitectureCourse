#include "Renderer.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer()
{

}


void Renderer::Init()
{
	//Init glad
	if (!gladLoadGL())
	{
		ThreadSafePrintf("Failed to load glad\n");
	}
	else
	{
		ThreadSafePrintf("Glad loaded successfully\n");
	}

	//Make sure opengl works
	const char* pRenderer = (const char*)glGetString(GL_RENDERER);
	const char* pVersion = (const char*)glGetString(GL_VERSION);
	ThreadSafePrintf("Renderer: %s\nVersion: %s\n", pRenderer, pVersion);

	//Setup opengl to be CCW
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//Create a standard texture for the shaders when not using a texture (Avoid branching)
	sf::Uint8 pixels[4] = { 255, 255, 255, 255 };
	m_WhiteTexture.create(1, 1);
	m_WhiteTexture.update(pixels);

	//Init shaders
	std::string vs = R"(
			#version 120
			
			attribute vec3 a_Position;
			attribute vec3 a_Normal;
			attribute vec3 a_Tangent;
			attribute vec2 a_TexCoord;

			uniform mat4 u_Transform;
			uniform mat4 u_Projection;
			uniform mat4 u_View;

			varying vec3 v_Position;
			varying vec3 v_Normal;
			varying vec3 v_Tangent;
			varying vec2 v_TexCoord;

			void main()
			{
				v_Position	= a_Position;
				v_Normal	= a_Normal;
				v_Tangent	= a_Tangent;
				v_TexCoord	= a_TexCoord;
				gl_Position = u_Projection * u_View * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

	std::string fs = R"(
			#version 120
    
			varying vec3 v_Position;
			varying vec3 v_Normal;
			varying vec3 v_Tangent;
			varying vec2 v_TexCoord;

			uniform vec4 u_Color;
			uniform sampler2D u_Texture;

			void main()
			{
				gl_FragColor = texture2D(u_Texture, v_TexCoord) * u_Color;
			}
		)";

	m_MeshShader.loadFromMemory(vs, fs);
	m_MeshShader.setUniform("u_Projection", sf::Glsl::Mat4(glm::value_ptr(glm::identity<glm::mat4>())));
	m_MeshShader.setUniform("u_View", sf::Glsl::Mat4(glm::value_ptr(glm::identity<glm::mat4>())));
	m_MeshShader.setUniform("u_Transform", sf::Glsl::Mat4(glm::value_ptr(glm::identity<glm::mat4>())));
	m_MeshShader.setUniform("u_Color", sf::Glsl::Vec4(sf::Color::White));
	m_MeshShader.setUniform("u_Texture", m_WhiteTexture);
}


void Renderer::Begin(const sf::Color& color, const Camera& camera)
{
	//Clear explicit since window.clear may not clear depthbuffer?
	glClearColor(color.r, color.g, color.b, color.a);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set shader and camera
	sf::Shader::bind(&m_MeshShader);
	m_MeshShader.setUniform("u_Projection", sf::Glsl::Mat4(glm::value_ptr(camera.GetProjection())));
	m_MeshShader.setUniform("u_View", sf::Glsl::Mat4(glm::value_ptr(camera.GetView())));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}


void Renderer::Submit(Mesh* pMesh, Texture* pTexture, const glm::mat4& transform)
{
	Submit(pMesh, pTexture->GetSFTexture(), sf::Color::White, transform);
}


void Renderer::Submit(Mesh* pMesh, Texture* pTexture, const sf::Color& color, const glm::mat4& transform)
{
	Submit(pMesh, pTexture->GetSFTexture(), color, transform);
}


void Renderer::Submit(Mesh* pMesh, const sf::Color& color, const glm::mat4& transform)
{
	Submit(pMesh, m_WhiteTexture, color, transform);
}


void Renderer::Submit(Mesh* pMesh, const sf::Texture& texture, const sf::Color& color, const glm::mat4& transform)
{
	m_MeshShader.setUniform("u_Texture", texture);
	m_MeshShader.setUniform("u_Color", sf::Glsl::Vec4(color));
	m_MeshShader.setUniform("u_Transform", sf::Glsl::Mat4(glm::value_ptr(transform)));
	pMesh->Draw();
}


void Renderer::End()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	sf::Shader::bind(nullptr);
}
