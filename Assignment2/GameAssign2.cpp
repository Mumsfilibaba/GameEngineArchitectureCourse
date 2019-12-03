#include "GameAssign2.h"
#include "ResourceManager.h"
#include "ResourceBundle.h"
#include "TaskManager.h"
#include "LoaderOBJ.h"
#include "LoaderTGA.h"
#include "LoaderBMP.h"
#include "LoaderCOLLADA.h"
#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

void Func()
{
	for (uint32_t i = 0; i < 200000000; i++)
		i++;
}

void GameAssign2::Init()
{
	ResourceManager& resourceManager = ResourceManager::Get();

    //create package
    resourceManager.CreateResourcePackage({ "meme.tga", "Phone.tga", "teapot.obj", "bunny.obj", "BMPTest_24.bmp" });

    //load resources
    ResourceBundle* pBundle = resourceManager.LoadResources({"meme.tga", "BMPTest_24.bmp", "teapot.obj", "bunny.obj" });

	m_pTexture = pBundle->GetTexture("BMPTest_24.bmp");


	resourceManager.LoadResourcesInBackground({ "meme.tga" }, [](ResourceBundle* bundle)
	{
		std::cout << "Loaded meme.tga in background!" << std::endl;
		//Background Loaded
	});

	resourceManager.LoadResourcesInBackground({ "Phone.tga" }, [](ResourceBundle* bundle)
	{
		std::cout << "Loaded Phone.tga in background!" << std::endl;
		//Backgroun Loaded
	});

	//Construct mesh
	m_pCube = Mesh::CreateCube();
	m_pCube->Construct();

	m_pMesh = pBundle->GetMesh("teapot.obj");
	m_pMesh->Construct();

	m_pBunny = pBundle->GetMesh("bunny.obj");
	m_pBunny->Construct();

	LoaderCOLLADA::ReadFromDisk("bunny.dae");
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
	Renderer::Get().Submit(m_pBunny, sf::Color::Blue, glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f)));
	Renderer::Get().Submit(m_pMesh, sf::Color::Red, glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
	Renderer::Get().Submit(m_pCube, m_pTexture, glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 0.0f)));
}

void GameAssign2::RenderImGui()
{
}

void GameAssign2::Release()
{
	//Note: We are using our own defined delete (Look in Mesh.h) - We are not using the OS defined
	if (m_pCube)
	{
		delete m_pCube;
		m_pCube = nullptr;
	}

	if (m_pMesh)
	{
		delete m_pMesh;
		m_pMesh = nullptr;
	}

	if (m_pBunny)
	{
		delete m_pBunny;
		m_pBunny = nullptr;
	}
}
