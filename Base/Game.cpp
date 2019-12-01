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
	m_ClearColor = sf::Color::Cyan;

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
	glFrontFace(GL_CCW);

	//Init ImGui
	ImGui::SFML::Init(*m_pRenderWindow);

	//Init shaders
	std::string vs = R"(
			#version 120
			
			attribute vec3 a_Position;
			attribute vec3 a_Normal;
			attribute vec3 a_Tangent;
			attribute vec2 a_TexCoord;

			uniform mat4 u_Projection;
			uniform mat4 u_View;

			varying vec3 v_Position;
			varying vec3 v_Normal;
			varying vec3 v_Tangent;
			varying vec2 v_TexCoord;

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
			#version 120
    
			varying vec3 v_Position;
			varying vec3 v_Normal;
			varying vec3 v_Tangent;
			varying vec2 v_TexCoord;

			uniform sampler2D our_Texture;

			void main()
			{
				gl_FragColor = texture2D(our_Texture, v_TexCoord);
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

    Debugger::SetDebugState(false);
    
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
				m_Camera.SetAspect(event.size.width, event.size.height);
				m_Camera.CreateView();
			}
		}

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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
	Render();

	glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
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
