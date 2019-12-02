#pragma once
#include "Helpers.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Shader.hpp>

class Renderer
{
public:
	Renderer();
	~Renderer() = default;
	
	void Init();

	void Begin(const sf::Color& color, const Camera& camera);
	void Submit(Mesh* pMesh, Texture* pTexture, const glm::mat4& transform);
	void Submit(Mesh* pMesh, Texture* pTexture, const sf::Color& color, const glm::mat4& transform);
	void Submit(Mesh* pMesh, const sf::Color& color, const glm::mat4& transform);
	void End();
private:
	void Submit(Mesh* pMesh, const sf::Texture& texture, const sf::Color& color, const glm::mat4& transform);
	sf::Shader m_MeshShader;
	sf::Texture m_WhiteTexture;
public:
	inline static Renderer& Get()
	{
		static Renderer instance;
		return instance;
	}
};