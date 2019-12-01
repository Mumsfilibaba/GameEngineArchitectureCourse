#include "GameAssign2.h"
#include "ResourceManager.h"

#include "MeshLoader.h"
#include "ResourceLoader.h"
#include "LoaderTGA.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

void GameAssign2::Init()
{
	ResourceLoader& resourceLoader = ResourceLoader::Get();
	ResourceManager& resourceManager = ResourceManager::Get();

	//register loaders
	resourceLoader.RegisterLoader(".tga", new LoaderTGA());


void GameAssign2::Init()
{
	//texture = txtrManager.LoadTGAFile("meme.tga");
    //Memory manager test
	//create package
	resourceManager.CreateResourcePackage({ "meme.tga", "Phone.tga" });
	
	//load resources
	ResourceBundle* pBundle = resourceManager.LoadResources({ "meme.tga", "Phone.tga" });

	m_pTexture = pBundle->GetTexture("Phone.tga");

#ifndef MACOS

	//size_t testTextureHash = HashString("our texture");
	//size_t testTextureSize = archiver.ReadRequiredSizeForPackageData(testTextureHash);

	////test texture
	//void* decompressedTestTextureData = MemoryManager::GetInstance().Allocate(testTextureHash, 1, "Test Texture Decompressed");
	//archiver.ReadPackageData(testTextureHash, decompressedTestTextureData, testTextureSize);
	//texture = txtrManager.LoadTGAFile(decompressedTestStringData);

#endif

	//Construct mesh
	m_pCube = Mesh::CreateCube();
	m_pCube->Construct();

	m_pMesh = MeshLoader::LoadMesh("teapot.obj");
	m_pMesh->Construct();
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
	m_MeshShader.setUniform("our_Texture", *texture);
	m_pMesh->Draw();
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
