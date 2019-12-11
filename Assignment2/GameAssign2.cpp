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
#define IN_USE 2
#define ONLY_LOADED 0
#define NOT_LOADED 1

const std::string PACKAGE_HEADER_PATH = "PackageHeader.txt";

std::string g_stateChangeName = "";
static int g_selectedState = -1;
void GameAssign2::RenderResourceDataInfo()
{
	ImVec4 notLoaded = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 isLoadedAndUsed = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 isLoadedNotUsed = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 color;

	ResourceManager* manager = &ResourceManager::Get();
	std::vector<IResource*> resourcesInUses;
	ResourceManager::Get().GetResourcesInUse(resourcesInUses);
	
	std::map<std::string, int> resourceStates;

	for (auto file : m_ResourcesInCompressedPackage)
	{
		if (manager->IsResourceLoaded(file))
		{
			IResource* res = manager->Get().GetResource(file);
			if (res->GetRefCount() > 0)
				resourceStates[file] = IN_USE;
			else
				resourceStates[file] = ONLY_LOADED;
		}
		else
			resourceStates[file] = NOT_LOADED;
	}

	ImGui::Begin("Resource Data Window");
	ImGui::Separator();

	constexpr int nrCount = 90;
	static float nrOfResources[nrCount] = { 0 };
	static const char* states[] = { "Load", "Unload", "Use",};
	nrOfResources[0] = resourcesInUses.size();


	ImGui::Text("Number of kilobytes currently in use");
	char buf[32];
	sprintf(buf, "%.2f/%.2f", (float)manager->GetUsedMemory() / 1024, (float)manager->GetMaxMemory() / 1024);
	ImGui::ProgressBar(((float)manager->GetUsedMemory() / (float)manager->GetMaxMemory()), ImVec2(0.0f, 0.0f), buf);

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
		for (int newState = 0; newState < IM_ARRAYSIZE(states); newState++)
		{
			if (ImGui::Selectable(states[newState]))
			{
				int currentState = resourceStates[g_stateChangeName];
				if(currentState == newState)
					break;

				switch (newState)
				{
				case NOT_LOADED:
					UnLoadResource(g_stateChangeName);
					break;
				case ONLY_LOADED:
					if (currentState == NOT_LOADED)
						LoadResource(g_stateChangeName);
					else if (currentState == IN_USE)
						UnUseResource(g_stateChangeName);
					break;
				case IN_USE:
					UseResource(g_stateChangeName);
					break;
				}
			}
		}
		ImGui::EndPopup();
	}

	while (it != resourceStates.end())
	{
		switch (it->second)
		{
		case NOT_LOADED:
			color = notLoaded;
			break;
		case ONLY_LOADED:
			color = isLoadedNotUsed;
			break;
		case IN_USE:
			color = isLoadedAndUsed;
			break;
		}

		if (manager->IsResourceLoaded(it->first))
		{
			IResource* entity = manager->GetResource(it->first);
			if (ImGui::Button(entity->GetName().c_str()))
			{
				//name keeps track of which entity was selected, I don't think ImGui know which button was pressed. 
				g_stateChangeName = entity->GetName();
				ImGui::OpenPopup("ResourceGroup");

			}ImGui::NextColumn();
			//ImGui::TextColored(color, entity->GetName().c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, std::to_string(entity->GetSize() / 1024.0f).c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, std::to_string(entity->GetRefCount()).c_str()); ImGui::NextColumn();
			ImGui::TextColored(color, std::to_string(entity->GetGUID()).c_str()); ImGui::NextColumn();
		}
		else
		{
			if (ImGui::Button(it->first.c_str()))
			{
				//name keeps track of which entity was selected, I don't think ImGui know which button was pressed. 
				g_stateChangeName = it->first;
				ImGui::OpenPopup("ResourceGroup");

			}ImGui::NextColumn();
			//ImGui::Button(it->first.c_str()); ImGui::NextColumn();
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

	while (!packageHeader.eof())
	{
		std::string resource;
		packageHeader >> resource;

		if (resource.length() > 0)
			m_ResourcesInCompressedPackage.push_back(resource);
	}

#endif
}

void GameAssign2::LoadResource(const std::string& file)
{
	if (m_Resources[file])
		return;

	ResourceManager::Get().LoadResourcesInBackground({ file.c_str() }, [this, file](const Ref<ResourceBundle>& bundle)
	{
		if (bundle)
		{
			m_Resources[file] = ResourceManager::Get().GetResource(file);
		}
	});
}

void GameAssign2::UnLoadResource(const std::string& file)
{
	m_Resources[file] = nullptr;
	UnUseResource(file);

	ResourceManager::Get().UnloadResource(HashString(file.c_str()));
}

void GameAssign2::UseResource(const std::string& file)
{
	if (m_Resources[file])
	{
		if (m_ResourcesInUse[file] != m_Resources[file])
		{
			m_ResourcesInUse[file] = m_Resources[file];
			m_ResourcesInUse[file]->AddRef();
		}
		return;
	}

	ResourceManager::Get().LoadResourcesInBackground({ file.c_str() }, [this, file](const Ref<ResourceBundle>& bundle)
	{
		if (bundle)
		{
			m_Resources[file] = ResourceManager::Get().GetResource(file);
			m_ResourcesInUse[file] = m_Resources[file];
			m_ResourcesInUse[file]->AddRef();
		}
	});
}

void GameAssign2::UnUseResource(const std::string& file)
{
	if (m_ResourcesInUse[file])
	{
		m_ResourcesInUse[file]->RemoveRef();
		m_ResourcesInUse[file] = nullptr;
	}
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
#if !defined(CREATE_PACKAGE)
	if (m_ResourcesInUse["bunny.obj"])
		Renderer::Get().Submit((Mesh*)m_ResourcesInUse["bunny.obj"], sf::Color::Green, glm::translate(glm::identity<glm::mat4>(), glm::vec3(2.0f, 0.0f, 0.0f)));

	if (m_ResourcesInUse["teapot.obj"])
		Renderer::Get().Submit((Mesh*)m_ResourcesInUse["teapot.obj"], sf::Color::Red, glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
    
	if (m_ResourcesInUse["cube.dae"] && m_ResourcesInUse["meme.tga"])
        Renderer::Get().Submit((Mesh*)m_ResourcesInUse["cube.dae"], (Texture*)m_ResourcesInUse["meme.tga"], glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 0.0f)));
    
	if (m_ResourcesInUse["M4A1.dae"])
    {
        glm::mat4 translation   = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 0.0f, 4.5f));
        glm::mat4 rotation      = glm::rotate(translation, glm::radians<float>(90), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 scale         = glm::scale(rotation, glm::vec3(0.15f, 0.15f, 0.15f));
        Renderer::Get().Submit((Mesh*)m_ResourcesInUse["M4A1.dae"], sf::Color::Blue, scale);
    }

	if (m_ResourcesInUse["AudiR8.dae"])
	{
		glm::mat4 translation	= glm::translate(glm::identity<glm::mat4>(), glm::vec3(-2.0f, 1.0f, -4.5f));
		glm::mat4 rotation = glm::rotate(translation, glm::radians<float>(90), glm::vec3(0.0f, 0.0f, 1.0f));
		rotation = glm::rotate(rotation, glm::radians<float>(90), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 scale			= glm::scale(rotation, glm::vec3(1.0f, 1.0f, 1.0f));
		Renderer::Get().Submit((Mesh*)m_ResourcesInUse["AudiR8.dae"], sf::Color::White, scale);
	}

	if (m_ResourcesInUse["stormtrooper.obj"] && m_ResourcesInUse["stormtrooper.tga"])
	{
		glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 2.0f));
		glm::mat4 rotation = glm::rotate(translation, glm::radians<float>(180), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 scale = glm::scale(rotation, glm::vec3(0.01f, 0.01f, 0.01f));
		Renderer::Get().Submit((Mesh*)m_ResourcesInUse["stormtrooper.obj"], (Texture*)m_ResourcesInUse["stormtrooper.tga"], scale);
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
	RenderResourceDataInfo();
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
