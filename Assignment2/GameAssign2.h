#pragma once
#include "Game.h"
#include "TextureManager.h"
#include "Texture.h"

#include <iostream>

#include "Archiver.h"
#include "StringHash.h"

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
	Texture* m_pTexture;
	Mesh* m_pCube;
};