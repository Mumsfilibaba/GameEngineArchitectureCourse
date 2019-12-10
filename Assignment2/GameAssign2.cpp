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
#include <imgui.h>
#include <filesystem>


//#define CREATE_PACKAGE

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
    resourceManager.CreateResourcePackage({ "meme.tga", "Phone.tga", "teapot.obj", "bunny.obj", "bunny.dae", "cube.dae", "M4A1.dae", "BMPTest_24.bmp" });

#if defined(CREATE_PACKAGE)
	std::string path = "Resources";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string fileNameString = entry.path().filename().string();
		char* fileName = new char[fileNameString.length() + 1];
		strcpy(fileName, fileNameString.c_str());
		m_ResourcesNotInPackage.push_back(fileName);
	}
#else
    //load resources
	Ref<ResourceBundle> pBundle = resourceManager.LoadResources({ "BMPTest_24.bmp", "teapot.obj", "bunny.obj", "bunny.dae", "cube.dae", "M4A1.dae" });
	pBundle = resourceManager.LoadResources({ "BMPTest_24.bmp", "teapot.obj", "bunny.obj", "bunny.dae", "cube.dae", "M4A1.dae" });

	resourceManager.LoadResourcesInBackground({ "meme.tga" }, [this](const Ref<ResourceBundle>& bundle)
	{
        ThreadSafePrintf("Loaded meme.tga in background!\n");
        m_pTexture = bundle.Get()->GetTexture("meme.tga");
	});

	resourceManager.LoadResourcesInBackground({ "Phone.tga" }, [this](const Ref<ResourceBundle>& bundle)
	{
        ThreadSafePrintf("Loaded Phone.tga in background!\n");
	});
    
    resourceManager.LoadResourcesInBackground({ "teapot.obj" }, [this](const Ref<ResourceBundle>& bundle)
    {
        ThreadSafePrintf("Loaded teapot.obj  in background!\n");
        m_pMesh = bundle.Get()->GetMesh("teapot.obj");
    });
    
    resourceManager.LoadResourcesInBackground({ "bunny.obj" }, [this](const Ref<ResourceBundle>& bundle)
    {
        ThreadSafePrintf("Loaded bunny.obj in background!\n");
        m_pBunny = bundle.Get()->GetMesh("bunny.obj");
    });
    
    resourceManager.LoadResourcesInBackground({ "bunny.dae" }, [this](const Ref<ResourceBundle>& bundle)
    {
        ThreadSafePrintf("Loaded bunny.dae  in background!\n");
        //m_pCube = bundle.Get()->GetMesh("bunny.dae");
    });
    
    resourceManager.LoadResourcesInBackground({ "cube.dae" }, [this](const Ref<ResourceBundle>& bundle)
    {
        ThreadSafePrintf("Loaded cube.dae in background!\n");
        m_pCube = bundle.Get()->GetMesh("cube.dae");
    });
    
    resourceManager.LoadResourcesInBackground({ "M4A1.dae" }, [this](const Ref<ResourceBundle>& bundle)
    {
        ThreadSafePrintf("Loaded M4A1.dae in background!\n");
        m_pGun = bundle.Get()->GetMesh("M4A1.dae");
    });
#endif
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
#if !defined(CREATE_PACKAGE)
	if (m_pBunny && m_pTexture)
		Renderer::Get().Submit(m_pBunny.Get(), sf::Color::Green, glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f)));

	if (m_pMesh)
		Renderer::Get().Submit(m_pMesh.Get(), sf::Color::Red, glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
    
    if (m_pCube)
        Renderer::Get().Submit(m_pCube.Get(), m_pTexture.Get(), glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 0.0f)));
    
    if (m_pGun)
    {
        glm::mat4 translation   = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 2.5f));
        glm::mat4 rotation      = glm::rotate(translation, glm::radians<float>(90), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 scale         = glm::scale(rotation, glm::vec3(0.15f, 0.15f, 0.15f));
        Renderer::Get().Submit(m_pGun.Get(), sf::Color::Blue, scale);
    }
#endif
}

void GameAssign2::RenderImGui()
{
#if defined(CREATE_PACKAGE)
	if (ImGui::Begin("Package Creation Window"))
	{
		static bool packageSaved = false;
		static size_t packageSavedCounter = 0;

		ImGui::BeginChild("", ImVec2(ImGui::GetWindowWidth(), 20));
		if (m_ResourcesInPackage.size() > 0)
		{
			if (ImGui::Button("Create Package", ImVec2(120, 20)))
			{
				//create package
				ResourceManager::Get().CreateResourcePackage(m_ResourcesInPackage);

				std::ofstream fileStream;
				fileStream.open("resourcesSavedDebug.txt", std::ios_base::out);

				if (fileStream.is_open())
				{
					for (auto& str : m_ResourcesInPackage)
					{
						fileStream << str << std::endl;
					}

					fileStream.close();
				}

				packageSaved = true;
			}
		}
			
		if (packageSaved)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Package Saved!");
			packageSavedCounter++;

			if (packageSavedCounter >= 1000)
			{
				packageSaved = false;
				packageSavedCounter = 0;
			}
		}
		ImGui::EndChild();

		ImGui::Separator();
		ImGui::Columns(3, "Select Resources", true);

		constexpr size_t listsWidth = 300;

		ImGui::SetColumnWidth(0, listsWidth + 10);
		ImGui::SetColumnWidth(1, 34);
		ImGui::SetColumnWidth(2, listsWidth + 10);
		{
			static int currentNotInPackageItem = 0;
			static int currentInPackageItem = 0;

			ImGui::Text("Available Resources");
			ImGui::PushItemWidth(listsWidth);
			ImGui::PushID(0);
			ImGui::ListBox("", &currentNotInPackageItem, m_ResourcesNotInPackage.data(), m_ResourcesNotInPackage.size());
			ImGui::PopID();
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::NewLine();
			if (ImGui::ArrowButton("Move Right", ImGuiDir_Right))
			{
				if (m_ResourcesNotInPackage.size() > 0)
				{
					m_ResourcesInPackage.push_back(m_ResourcesNotInPackage[currentNotInPackageItem]);
					m_ResourcesNotInPackage.erase(m_ResourcesNotInPackage.begin() + currentNotInPackageItem);

					if (currentNotInPackageItem >= m_ResourcesNotInPackage.size() && m_ResourcesNotInPackage.size() > 0)
						currentNotInPackageItem--;
				}
			}

			if (ImGui::ArrowButton("Move Left", ImGuiDir_Left))
			{
				if (m_ResourcesInPackage.size() > 0)
				{
					m_ResourcesNotInPackage.push_back(m_ResourcesInPackage[currentInPackageItem]);
					m_ResourcesInPackage.erase(m_ResourcesInPackage.begin() + currentInPackageItem);

					if (currentInPackageItem >= m_ResourcesInPackage.size() && m_ResourcesInPackage.size() > 0)
						currentInPackageItem--;
				}
			}
			ImGui::NextColumn();

			
			ImGui::Text("Resources in Package");
			ImGui::PushItemWidth(listsWidth);
			ImGui::PushID(1);
			ImGui::ListBox("", &currentInPackageItem, m_ResourcesInPackage.data(), m_ResourcesInPackage.size());
			ImGui::PopID();
			ImGui::PopItemWidth();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);
		ImGui::End();
	}
#endif
}

void GameAssign2::Release()
{
	for (size_t i = 0; i < m_ResourcesNotInPackage.size(); i++)
	{
		delete[] m_ResourcesNotInPackage[i];
	}

	for (size_t i = 0; i < m_ResourcesInPackage.size(); i++)
	{
		delete[] m_ResourcesInPackage[i];
	}
}
