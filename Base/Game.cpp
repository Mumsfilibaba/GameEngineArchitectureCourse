#include "Game.h"
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include "Debugger.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4002)		//Disable: "too many arguments for function-like macro invocation"-warning
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

void Game::InternalInit()
{
	MEMLEAKCHECK;

	m_ClearColor = sf::Color::Black;
	m_pRenderWindow = pool_new(sf::RenderWindow, "Main Window") sf::RenderWindow(sf::VideoMode(1280, 720), "Game Engine Architecture");
	m_pRenderWindow->setVerticalSyncEnabled(false);
	m_pRenderWindow->setFramerateLimit(0);

	ImGui::SFML::Init(*m_pRenderWindow);

	Init();
}

void Game::Run()
{
	InternalInit();

	sf::Clock deltaClock;
	while (m_pRenderWindow->isOpen())
	{
		sf::Event event;
		while (m_pRenderWindow->pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			if (event.type == sf::Event::Closed)
			{
				m_pRenderWindow->close();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				{
					m_pRenderWindow->close();
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
				{
					Debugger::SetDebugState(!Debugger::GetDebugState());
				}
			}
		}

		sf::Time deltaTime = deltaClock.restart();
		InternalUpdate(deltaTime);

		m_pRenderWindow->clear(m_ClearColor);

		InternalRender(deltaTime);
		InternalRenderImGui(deltaTime);

		m_pRenderWindow->display();
	}

	InternalRelease();
}

void Game::InternalUpdate(const sf::Time& deltatime)
{
	Update(deltatime);
	ImGui::SFML::Update(*m_pRenderWindow, deltatime);
}

void Game::InternalRender(const sf::Time& deltatime)
{
	Render();
}

void Game::InternalRenderImGui(const sf::Time& deltatime)
{
	Debugger::DrawDebugUI(deltatime);
	
	RenderImGui();
	ImGui::SFML::Render(*m_pRenderWindow);
}

void Game::InternalRelease()
{
	Release();

	ImGui::SFML::Shutdown();

	pool_delete(m_pRenderWindow);
	m_pRenderWindow = nullptr;
}
