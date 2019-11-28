#include "Game.h"
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <glad/glad.h>
#include "Debugger.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4002)		//Disable: "too many arguments for function-like macro invocation"-warning
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

void Game::InternalInit()
{
	MEMLEAKCHECK;

	//Init clearcolor
	m_ClearColor = sf::Color::Black;

	//Init window
	sf::ContextSettings settings;
	settings.depthBits			= 24;
	settings.stencilBits		= 8;
#ifdef DEBUG
	settings.attributeFlags		= sf::ContextSettings::Debug;
#else
	settings.attributeFlags		= 0;
#endif
	settings.antialiasingLevel	= 4;
	settings.majorVersion		= 3;
	settings.minorVersion		= 3;

	m_pRenderWindow = pool_new(sf::RenderWindow, "Main Window") sf::RenderWindow(sf::VideoMode(1280, 720), "Game Engine Architecture", sf::Style::Default, settings);
	m_pRenderWindow->setVerticalSyncEnabled(false);
	m_pRenderWindow->setFramerateLimit(0);

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
	ThreadSafePrintf("Renderer: %s\nVersion: %s\nMSAA: %dx\n", pRenderer, pVersion, settings.antialiasingLevel);

	//Init ImGui
	ImGui::SFML::Init(*m_pRenderWindow);

	//Init shaders
	std::string vs = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec3 a_Normal;
			layout(location = 2) in vec3 a_Tangent;
			layout(location = 3) in vec2 a_TexCoord;

			out vec3 v_Position;
			out vec3 v_Normal;
			out vec3 v_Tangent;
			out vec2 v_TexCoord;

			void main()
			{
				v_Position	= a_Position;
				v_Normal	= a_Normal;
				v_Tangent	= a_Tangent;
				v_TexCoord	= a_TexCoord;
				gl_Position = vec4(a_Position, 1.0);	
			}
		)";

	std::string fs = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec3 v_Normal;
			in vec3 v_Tangent;
			in vec2 v_TexCoord;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
			}
		)";

	m_MeshShader.loadFromMemory(vs, fs);

	//Init client
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
			else if (event.type == sf::Event::Resized)
			{
				glViewport(0, 0, event.size.width, event.size.height);
			}
		}

		sf::Time deltaTime = deltaClock.restart();
		InternalUpdate(deltaTime);

		//Clear explicit since window.clear may not clear depthbuffer?
		glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	Update(deltatime);
	ImGui::SFML::Update(*m_pRenderWindow, deltatime);
}

void Game::InternalRender(const sf::Time& deltatime)
{
	sf::Shader::bind(&m_MeshShader);

	Render();

	sf::Shader::bind(nullptr);
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
