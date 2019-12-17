#pragma once
#include <iostream>
#include "Game.h"
#include "ResourceBundle.h"
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

	void LoadResource(const std::string& file);
	void UnLoadResource(const std::string& file);
	void UseResource(const std::string& file);
	void UnUseResource(const std::string& file);

	void RenderResourceDataInfo();
	void ChangeStateOfResource(const std::string& file, int state, const std::map<std::string, int>& resourceStates);
	void GameAssign2::GetCurrentState(std::map<std::string, int>& resourceStates);

private:
	std::vector<std::string> m_ResourcesInCompressedPackage;

	std::vector<char*> m_ResourcesNotInPackage;
	std::vector<char*> m_ResourcesInPackage;
	bool m_StressTest;
	int m_Timer;

	void SingleThreadedTest();
	void MultiThreadedTest();
};
