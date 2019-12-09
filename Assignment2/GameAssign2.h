#pragma once
#include <iostream>
#include "Game.h"
#include "TextureManager.h"
#include "Texture.h"
#include "Ref.h"

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
	Ref<Texture> m_pTexture;
	Ref<Mesh> m_pCube;
	Ref<Mesh> m_pMesh;
	Ref<Mesh> m_pBunny;
	static GameAssign2* s_pInstance;
};