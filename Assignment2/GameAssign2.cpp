#include "GameAssign2.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"
#include "TaskManager.h"
#include "LoaderOBJ.h"
#include "LoaderTGA.h"
#include "LoaderBMP.h"
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
	ResourceLoader& resourceLoader = ResourceLoader::Get();
	ResourceManager& resourceManager = ResourceManager::Get();

	//register loaders
	resourceLoader.RegisterLoader(".tga", new LoaderTGA());
	resourceLoader.RegisterLoader(".bmp", new LoaderBMP());
    resourceLoader.RegisterLoader(".obj", new LoaderOBJ());

    //create package
    resourceManager.CreateResourcePackage({ "meme.tga", "Phone.tga", "teapot.obj", "BMPTest_24.bmp" });

    //load resources
    ResourceBundle* pBundle = resourceManager.LoadResources({ "meme.tga", "Phone.tga", "teapot.obj", "BMPTest_24.bmp"  });

	m_pTexture = pBundle->GetTexture("BMPTest_24.bmp");


	resourceManager.LoadResourcesInBackground({});

#ifndef MACOS

    //size_t testTextureHash = HashString("our texture");
    //size_t testTextureSize = archiver.ReadRequiredSizeForPackageData(testTextureHash);

    ////test texture
    //void* decompressedTestTextureData = MemoryManager::GetInstance().Allocate(testTextureHash, 1, "Test Texture Decompressed");
    //archiver.ReadPackageData(testTextureHash, decompressedTestTextureData, testTextureSize);
    //texture = txtrManager.LoadTGAFile(decompressedTestStringData);

#endif

	TaskManager& taskManager = TaskManager::Get();
	taskManager.Execute(Func);
	taskManager.Execute(Func);
	taskManager.Execute(Func);
	taskManager.Execute(Func);
	taskManager.Execute(Func);
	taskManager.Execute(Func);

	//Construct mesh
	m_pCube = Mesh::CreateCube();
	m_pCube->Construct();

	m_pMesh = pBundle->GetMesh("teapot.obj");
	m_pMesh->Construct();
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
	Renderer::Get().Submit(m_pCube, m_pTexture, glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f)));
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
}
