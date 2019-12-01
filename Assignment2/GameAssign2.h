#pragma once
#include <iostream>
#include "Game.h"
#include "TextureManager.h"
#include "Archiver.h"

class GameAssign2 final : public Game
{
public:
	GameAssign2() = default;
	~GameAssign2() = default;

	virtual void Init() override final;
	virtual void Update(const sf::Time& deltaTime) override final;
	virtual void Render() override final;
	virtual void RenderImGui() override final;
	virtual void Release() override final;
private:
	TextureManager txtrManager;
	sf::Texture* texture;
	Mesh* m_pCube;
	Mesh* m_pMesh;
};