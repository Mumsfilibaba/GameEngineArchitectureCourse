#include "GameAssign2.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

void GameAssign2::Init()
{
	txtrManager.LoadTGAFile("flag_b16.tga", txtrManager.m_tgaFile);
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
}

void GameAssign2::RenderImGui()
{
}

void GameAssign2::Release()
{
}
