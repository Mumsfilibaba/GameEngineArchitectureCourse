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

GameAssign2* GameAssign2::s_pInstance = nullptr;

void Func()
{
	for (uint32_t i = 0; i < 200000000; i++)
		i++;
}

void GameAssign2::Init()
{
	s_pInstance = this;
	ResourceManager& resourceManager = ResourceManager::Get();

    //create package
    resourceManager.CreateResourcePackage({ "meme.tga", "Phone.tga", "teapot.obj", "bunny.obj", "BMPTest_24.bmp" });

    //load resources
	Ref<ResourceBundle> pBundle = resourceManager.LoadResources({"BMPTest_24.bmp", "teapot.obj", "bunny.obj" });
	pBundle = resourceManager.LoadResources({ "BMPTest_24.bmp", "teapot.obj", "bunny.obj" });

	m_pTexture = pBundle.Get()->GetTexture("BMPTest_24.bmp");


	resourceManager.LoadResourcesInBackground({ "meme.tga" }, [](const Ref<ResourceBundle>& bundle)
	{
		std::cout << "Loaded meme.tga in background!" << std::endl;
		//Background Loaded
	});

	resourceManager.LoadResourcesInBackground({ "Phone.tga" }, [](const Ref<ResourceBundle>& bundle)
	{
		std::cout << "Loaded Phone.tga in background!" << std::endl;
		s_pInstance->m_pTexture = bundle.Get()->GetTexture("Phone.tga");
		//Backgroun Loaded
	});

    
	//Construct meshes
    m_pMesh = pBundle.Get()->GetMesh("teapot.obj");
	m_pBunny = pBundle.Get()->GetMesh("bunny.obj");


	LoaderCOLLADA::ReadFromDisk("bunny.dae");
}

void onLoaded(const Ref<ResourceBundle>& bundle)
{
	std::cout << "Loaded meme.tga in background!" << std::endl;
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
	if (m_pBunny && m_pTexture)
		Renderer::Get().Submit(m_pBunny.Get(), m_pTexture.Get(), glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f)));

	if (m_pMesh)
		Renderer::Get().Submit(m_pMesh.Get(), sf::Color::Red, glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
}

void GameAssign2::RenderImGui()
{
}

void GameAssign2::Release()
{
	
}
