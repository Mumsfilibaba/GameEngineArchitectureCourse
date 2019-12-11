#include "GameAssign2.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"
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

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

//#define CREATE_PACKAGE
#ifdef CREATE_PACKAGE
const std::string UNPACKAGED_RESOURCES_DIR = "Resources";
#endif

#define RESOURCE_INFO_DEBUG

const std::string PACKAGE_HEADER_PATH = "PackageHeader.txt";

void RenderResourceDataInfo(Ref<ResourceBundle>& pBundle, std::vector<std::string> resourceInPackage)
{
	ImVec4 notLoaded = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 isLoadedAndUsed = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 isLoadedNotUsed = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 color;

	ResourceManager* manager = &ResourceManager::Get();
	std::vector<IResource*> resourcesInUses;
	std::string name = "";
	ResourceManager::Get().GetResourcesInUse(resourcesInUses);
	
	std::map<std::string, int> resourceStates;

	for (auto file : resourceInPackage)
	{
		if (manager->IsResourceLoaded(file))
		{
			IResource* res = manager->Get().GetResource(HashString(file.c_str()));
			if (res->GetRefCount() > 0)
				resourceStates[file] = 3;
			else
				resourceStates[file] = 2;
		}
		else
			resourceStates[file] = 1;
	}

	ImGui::ShowDemoWindow();
	ImGui::Begin("Resource Data Window");
	ImGui::Separator();

	constexpr int nrCount = 90;
	static float nrOfResources[nrCount] = { 0 };
	static int   valuesOffset = 0;
	static int selectedState = -1;
	const char* states[] = { "Load", "Unload", "Use",};
	nrOfResources[valuesOffset] = ResourceManager::Get().GetNrOfResourcesInUse();
	valuesOffset = (valuesOffset + 1) % nrCount;

	ImGui::Text("Number of Resources in use: %d", ResourceManager::Get().GetNrOfResourcesInUse());
	ImGui::PlotLines("", nrOfResources, 90, 0, "", 0.0f, 30.0f, ImVec2(0, 80));

	ImGui::Separator();

	ImGui::Columns(4, "Resources being referenced", true);

	ImGui::Text("name:");
	ImGui::NextColumn();

	ImGui::Text("Size(kb):");
	ImGui::NextColumn();

	ImGui::Text("References:");
	ImGui::NextColumn();

	ImGui::Text("GUID:");
	ImGui::NextColumn();

	std::map<std::string, int>::iterator it = resourceStates.begin();

	if (ImGui::BeginPopup("ResourceGroup"))
	{
		ImGui::Text("Change State into:");
		ImGui::Separator();
		for (int i = 0; i < IM_ARRAYSIZE(states); i++)
		{
			if (ImGui::Selectable(states[i]))
			{
				selectedState = i;
			}
		}
		ImGui::EndPopup();
	}

	while (it != resourceStates.end())
	{
		switch (it->second)
		{
		case 1:
			color = notLoaded;
			break;
		case 2:
			color = isLoadedNotUsed;
			break;
		case 3:
			color = isLoadedAndUsed;
			break;
		}

		IResource* entity = manager->GetResource(HashString(it->first.c_str()));
		if (ImGui::Button(entity->GetName().c_str()))
		{
			//name keeps track of which entity was selected, I don't think ImGui know which button was pressed. 
			name = entity->GetName();
			ImGui::OpenPopup("ResourceGroup");

		}ImGui::NextColumn();

		if (entity)
		{

			//ImGui::TextColored(color, entity->GetName().c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, std::to_string(entity->GetSize() / 1024.0f).c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, std::to_string(entity->GetRefCount()).c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, std::to_string(entity->GetGUID()).c_str()); ImGui::NextColumn();
		}
		else
		{
			ImGui::Button(it->first.c_str()); ImGui::NextColumn();
			//ImGui::TextColored(color, it->first.c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, "No Data"); ImGui::NextColumn();
			ImGui::TextColored(color, "No Data"); ImGui::NextColumn();
			ImGui::TextColored(color, "No Data"); ImGui::NextColumn();
		}

		it++;
	}

	ImGui::End();
}

void GameAssign2::Init()
{
	ResourceManager& resourceManager = ResourceManager::Get();

#if defined(CREATE_PACKAGE)
	for (const auto& entry : std::filesystem::directory_iterator(UNPACKAGED_RESOURCES_DIR))
	{
		std::string fileNameString = entry.path().filename().string();

		if (ResourceLoader::Get().HasLoaderForFile(fileNameString))
		{
			char* fileName = new char[fileNameString.length() + 1];
			strcpy(fileName, fileNameString.c_str());
			m_ResourcesNotInPackage.push_back(fileName);
		}
	}
#else
	//Read Package Header created by the Packaging Tool
	std::ifstream packageHeader;
	packageHeader.open(PACKAGE_HEADER_PATH, std::ios_base::in);

	std::vector<std::string> resourcesInPackage;
	while (!packageHeader.eof())
	{
		std::string resource;
		packageHeader >> resource;

		if (resource.length() > 0)
			m_ResourcesInCompressedPackage.push_back(resource);
	}

	//Load Resources described in the Package Header
	m_pBundle = resourceManager.LoadResources(m_ResourcesInCompressedPackage);
	//m_pBundle = resourceManager.LoadResources({ "BMPTest_24.bmp", "teapot.obj", "bunny.obj", "bunny.dae", "cube.dae", "M4A1.dae" });

	resourceManager.LoadResourcesInBackground({ "meme.tga" }, [this](const Ref<ResourceBundle>& bundle)
	{
		if (bundle)
		{
			m_pTexture = bundle.Get()->GetTexture("meme.tga");
		}
	});

	resourceManager.LoadResourcesInBackground({ "Phone.tga" }, [this](const Ref<ResourceBundle>& bundle)
	{
       
	});
    
    resourceManager.LoadResourcesInBackground({ "teapot.obj" }, [this](const Ref<ResourceBundle>& bundle)
    {
		if (bundle)
		{
			m_pMesh = bundle.Get()->GetMesh("teapot.obj");
		}
    });
    
    resourceManager.LoadResourcesInBackground({ "bunny.obj" }, [this](const Ref<ResourceBundle>& bundle)
    {
		if (bundle)
		{
			m_pBunny = bundle.Get()->GetMesh("bunny.obj");
		}
    });
    
    resourceManager.LoadResourcesInBackground({ "bunny.dae" }, [this](const Ref<ResourceBundle>& bundle)
    {
		if (bundle)
		{
			//m_pCube = bundle.Get()->GetMesh("bunny.dae");
		}
    });
    
    resourceManager.LoadResourcesInBackground({ "cube.dae" }, [this](const Ref<ResourceBundle>& bundle)
    {
		if (bundle)
		{
			m_pCube = bundle.Get()->GetMesh("cube.dae");
		}
	});
    
    resourceManager.LoadResourcesInBackground({ "M4A1.dae" }, [this](const Ref<ResourceBundle>& bundle)
    {
		if (bundle)
		{
			m_pGun = bundle.Get()->GetMesh("M4A1.dae");
		}
    });

	resourceManager.LoadResourcesInBackground({ "AudiR8.dae" }, [this](const Ref<ResourceBundle>& bundle)
	{
		if (bundle)
		{
			m_pCar = bundle.Get()->GetMesh("AudiR8.dae");
		}
	});

	resourceManager.LoadResourcesInBackground({ "stormtrooper.obj" }, [this](const Ref<ResourceBundle>& bundle)
	{
		if (bundle)
		{
			m_pStorm = bundle.Get()->GetMesh("stormtrooper.obj");
		}
	});

	resourceManager.LoadResourcesInBackground({ "stormtrooper.tga" }, [this](const Ref<ResourceBundle>& bundle)
	{
		if (bundle)
		{
			m_pTexture2 = bundle.Get()->GetTexture("stormtrooper.tga");
		}
	});
#endif
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
#if !defined(CREATE_PACKAGE)
	if (m_pBunny)
		Renderer::Get().Submit(m_pBunny.Get(), sf::Color::Green, glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f)));

	if (m_pMesh)
		Renderer::Get().Submit(m_pMesh.Get(), sf::Color::Red, glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
    
    if (m_pCube && m_pTexture)
        Renderer::Get().Submit(m_pCube.Get(), m_pTexture.Get(), glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 0.0f)));
    
    if (m_pGun)
    {
        glm::mat4 translation   = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 4.5f));
        glm::mat4 rotation      = glm::rotate(translation, glm::radians<float>(90), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 scale         = glm::scale(rotation, glm::vec3(0.15f, 0.15f, 0.15f));
        Renderer::Get().Submit(m_pGun.Get(), sf::Color::Blue, scale);
    }

	if (m_pCar)
	{
		glm::mat4 translation	= glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 1.0f, -4.5f));
		glm::mat4 rotation = glm::rotate(translation, glm::radians<float>(90), glm::vec3(0.0f, 0.0f, 1.0f));
		rotation = glm::rotate(rotation, glm::radians<float>(90), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 scale			= glm::scale(rotation, glm::vec3(1.0f, 1.0f, 1.0f));
		Renderer::Get().Submit(m_pCar.Get(), sf::Color::White, scale);
	}

	if (m_pStorm && m_pTexture2)
	{
		glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 2.0f));
		glm::mat4 rotation = glm::rotate(translation, glm::radians<float>(180), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 scale = glm::scale(rotation, glm::vec3(0.01f, 0.01f, 0.01f));
		Renderer::Get().Submit(m_pStorm.Get(), m_pTexture2.Get(), scale);
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
				ResourceManager::Get().CreateResourcePackage(UNPACKAGED_RESOURCES_DIR + "/", m_ResourcesInPackage);

				std::ofstream fileStream;
				fileStream.open(PACKAGE_HEADER_PATH, std::ios_base::out);

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
#if defined(RESOURCE_INFO_DEBUG)
	RenderResourceDataInfo(m_pBundle, m_ResourcesInCompressedPackage);
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
