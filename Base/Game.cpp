#include "Game.h"
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Debugger.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"
#include "LoaderTGA.h"
#include "LoaderBMP.h"
#include "LoaderOBJ.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4002)		//Disable: "too many arguments for function-like macro invocation"-warning
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

void Game::InternalInit()
{
	MEMLEAKCHECK;

	//Init window
	sf::ContextSettings settings;
	settings.depthBits			= 24;
	settings.stencilBits		= 8;
    settings.attributeFlags     = 0;
#ifdef DEBUG
    settings.attributeFlags     |= sf::ContextSettings::Debug;
#endif
	settings.antialiasingLevel	= 4;
	settings.majorVersion		= 2;
	settings.minorVersion		= 1;

	m_pRenderWindow = pool_new(sf::RenderWindow, "Main Window") sf::RenderWindow(sf::VideoMode(1280, 720), "Game Engine Architecture", sf::Style::Default, settings);
	m_pRenderWindow->setVerticalSyncEnabled(false);
	m_pRenderWindow->setFramerateLimit(0);

	//Init renderer
	Renderer::Get().Init();

	//Init ImGui
	ImGui::SFML::Init(*m_pRenderWindow);

	//Init camera
	m_Camera.SetAspect(float(m_pRenderWindow->getSize().x), float(m_pRenderWindow->getSize().y));
	m_Camera.SetPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	m_Camera.CreateProjection();
	m_Camera.CreateView();

    Debugger::SetDebugState(false);
    
	ResourceLoader& resourceLoader = ResourceLoader::Get();
	resourceLoader.RegisterLoader(".tga", new LoaderTGA());
	resourceLoader.RegisterLoader(".bmp", new LoaderBMP());
	resourceLoader.RegisterLoader(".obj", new LoaderOBJ());

	//Init client
	Init();
}

void Game::Run()
{
	InternalInit();

	sf::Clock deltaClock;
	while (m_pRenderWindow->isOpen())
	{
        //Get deltatime of lastframe since we need it in the eventloop
        sf::Time deltaTime = deltaClock.restart();
        
        //Check all events
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
				if (event.key.code == sf::Keyboard::Escape)
				{
					m_pRenderWindow->close();
				}
				else if (event.key.code == sf::Keyboard::Num1)
				{
					Debugger::SetDebugState(!Debugger::GetDebugState());
					ThreadSafePrintf(Debugger::GetDebugState() ? "Debugger Enabled\n" : "Debugger Disabled\n");
				}
			}
			else if (event.type == sf::Event::Resized)
			{
				glViewport(0, 0, event.size.width, event.size.height);
				
				//Update camera
				m_Camera.SetAspect(float(event.size.width), float(event.size.height));
				m_Camera.CreateView();
			}
		}

		InternalUpdate(deltaTime);
       
		//Draw customs stuff
		InternalRender(deltaTime);

		//Draw SFML stuff
		m_pRenderWindow->pushGLStates();
		m_pRenderWindow->resetGLStates();
		InternalRenderImGui(deltaTime);
		m_pRenderWindow->popGLStates();

		//Swapbuffers
		m_pRenderWindow->display();
	}

	InternalRelease();
}

void Game::InternalUpdate(const sf::Time& deltatime)
{
	ImGui::SFML::Update(*m_pRenderWindow, deltatime);
	ResourceManager::Get().Update();
	Update(deltatime);
    
    //Move camera
    constexpr float speed = 2.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        m_Camera.Translate(glm::vec3(0.0f, 0.0f, speed) * deltatime.asSeconds());
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        m_Camera.Translate(glm::vec3(0.0f, 0.0f, -speed) * deltatime.asSeconds());

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        m_Camera.Translate(glm::vec3(speed, 0.0f, 0.0f) * deltatime.asSeconds());
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        m_Camera.Translate(glm::vec3(-speed, 0.0f, 0.0f) * deltatime.asSeconds());

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        m_Camera.Translate(glm::vec3(0.0f, speed, 0.0f) * deltatime.asSeconds());
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        m_Camera.Translate(glm::vec3(0.0f, -speed, 0.0f) * deltatime.asSeconds());

    //Rotate camera
    constexpr float rotation = 30.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        m_Camera.Rotate(glm::vec3(-rotation, 0.0f, 0.0f) * deltatime.asSeconds());
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        m_Camera.Rotate(glm::vec3(rotation, 0.0f, 0.0f) * deltatime.asSeconds());

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        m_Camera.Rotate(glm::vec3(0.0f, -rotation, 0.0f) * deltatime.asSeconds());
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        m_Camera.Rotate(glm::vec3(0.0f, rotation, 0.0f) * deltatime.asSeconds());

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        m_Camera.Rotate(glm::vec3(0.0f, 0.0f, rotation) * deltatime.asSeconds());
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        m_Camera.Rotate(glm::vec3(0.0f, 0.0f, -rotation) * deltatime.asSeconds());

    m_Camera.CreateView();
}

void Game::InternalRender(const sf::Time& deltatime)
{
	Renderer::Get().Begin(sf::Color::Red, m_Camera);
	Render();
	Renderer::Get().End();
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
