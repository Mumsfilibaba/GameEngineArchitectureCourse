#include "Game.h"
#include <iostream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
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

	//Setup opengl to be CCW
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	//Init ImGui
	ImGui::SFML::Init(*m_pRenderWindow);

	//Init shaders
	std::string vs = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec3 a_Normal;
			layout(location = 2) in vec3 a_Tangent;
			layout(location = 3) in vec2 a_TexCoord;

			uniform mat4 u_Projection;
			uniform mat4 u_View;

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
				gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);	
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

	//Init camera
	m_Camera.SetAspect(m_pRenderWindow->getSize().x, m_pRenderWindow->getSize().y);
	m_Camera.SetPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	m_Camera.CreateProjection();
	m_Camera.CreateView();

	m_MeshShader.setUniform("u_Projection", sf::Glsl::Mat4(glm::value_ptr(m_Camera.GetProjection())));
	m_MeshShader.setUniform("u_View",		sf::Glsl::Mat4(glm::value_ptr(m_Camera.GetView())));

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
				
				//Update camera
				m_Camera.SetAspect(event.size.width, event.size.height);
				m_Camera.CreateView();
			}
		}

		sf::Time deltaTime = deltaClock.restart();
		InternalUpdate(deltaTime);

		//Clear explicit since window.clear may not clear depthbuffer?
		glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
		glClearDepth(1.0);
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
	ImGui::SFML::Update(*m_pRenderWindow, deltatime);
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
	sf::Shader::bind(&m_MeshShader);
	m_MeshShader.setUniform("u_Projection", sf::Glsl::Mat4(glm::value_ptr(m_Camera.GetProjection())));
	m_MeshShader.setUniform("u_View", sf::Glsl::Mat4(glm::value_ptr(m_Camera.GetView())));

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
