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
				vec4 position = u_Transform * vec4(a_Position, 1.0);
				v_Position	= position.xyz;
				v_Normal	= a_Normal;
				v_Tangent	= a_Tangent;
				v_TexCoord	= a_TexCoord;
				gl_Position = u_Projection * u_View * position;
			}
		)";

	std::string fs = R"(
			#version 120
    
			varying vec3 v_Position;
			varying vec3 v_Normal;
			varying vec3 v_Tangent;
			varying vec2 v_TexCoord;

			uniform vec3 u_CameraPos;
			uniform vec4 u_Color;
			uniform sampler2D u_Texture;

			void main()
			{
				vec4 lightColor		= vec4(1.0, 1.0, 1.0, 1.0);
				vec3 lightPosition	= vec3(0.0, 2.0, 0.0);

				vec3 normal		= normalize(v_Normal);
				vec3 lightDir	= lightPosition - v_Position;  
				float distance	= length(lightDir);
				lightDir = (lightDir / distance);

				float diff			= max(dot(normal, lightDir), 0.0);
				float attenuation	= 1.0 / (distance);
				vec4 diffuse = diff * lightColor * attenuation;

				float specularStrength = 0.6;
				vec3 viewDir	= normalize(u_CameraPos - v_Position);
				vec3 reflectDir = reflect(-lightDir, normal);  

				float spec		= pow(max(dot(viewDir, reflectDir), 0.0), 128);
				vec4 specular	= specularStrength * spec * lightColor;  

				float ambientStrength = 0.1;
				vec4 ambient = ambientStrength * lightColor;

				gl_FragColor = (ambient + diffuse + specular) * (texture2D(u_Texture, v_TexCoord) * u_Color);
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
	
	auto pos = camera.GetPosition();
	m_MeshShader.setUniform("u_CameraPos", sf::Glsl::Vec3(pos.x, pos.y, pos.z));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
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
	//Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.getNativeHandle());
    
    auto uniformLocation = glGetUniformLocation(m_MeshShader.getNativeHandle(), "u_Texture");
	glUniform1i(uniformLocation, 0);

	//Bind color and transform
	m_MeshShader.setUniform("u_Color", sf::Glsl::Vec4(color));
	m_MeshShader.setUniform("u_Transform", sf::Glsl::Mat4(glm::value_ptr(transform)));
	
	//Draw
	pMesh->Draw();
}


void Renderer::End()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

	sf::Shader::bind(nullptr);
}
