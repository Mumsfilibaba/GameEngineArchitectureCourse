#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <sstream>
#include <thread>
#include "PoolAllocator.h"
#if defined(_WIN32)
    #include <crtdbg.h>
#endif
#include "StackAllocator.h"

#define MB(mb) mb * 1024 * 1024

#define USE_CUSTOM_ALLOCATOR

#ifdef USE_CUSTOM_ALLOCATOR
#define NEW_STACK stack_new
#define DELETE_STACK stack_delete
#define NEW_POOL(Type) pool_new(Type)
#define DELETE_POOL(Object) pool_new(Object)
#else
#define NEW_STACK new
#define DELETE_STACK delete
#define NEW_POOL(Type) new
#define DELETE_POOL(Object) delete Object
#endif

void ThreadSafePrintf(const char* pFormat, ...)
{
	static SpinLock printLock;
	std::lock_guard<SpinLock> lock(printLock);

	va_list args;
	va_start(args, pFormat);
	vprintf(pFormat, args);
	va_end(args);
}

void ImGuiPrintMemoryManagerAllocations()
{
	std::map<size_t, std::string> currentMemory = std::map<size_t, std::string>(MemoryManager::GetInstance().GetAllocations());
	const FreeEntry* pStartFreeEntry = MemoryManager::GetInstance().GetFreeList();
	const FreeEntry* pCurrentFreeEntry = pStartFreeEntry;

	size_t counter = 0;
	do
	{
		size_t currentAddress = (size_t)pCurrentFreeEntry;
		size_t nextAddress = (size_t)pCurrentFreeEntry->pNext;

		size_t mb = pCurrentFreeEntry->sizeInBytes / (1024 * 1024);
		size_t kb = (pCurrentFreeEntry->sizeInBytes - mb * (1024 * 1024)) / 1024;
		size_t bytes = (pCurrentFreeEntry->sizeInBytes - mb * (1024 * 1024) - kb * 1024);

		std::stringstream addressStream;
		addressStream << "Start Address: " << std::setw(25) << "0x" << std::hex << currentAddress << std::endl;
		addressStream << "End Address: " << std::setw(27) << "0x" << std::hex << (currentAddress + pCurrentFreeEntry->sizeInBytes - 1) << std::endl;
		addressStream << "Next Free Address: " << std::setw(21) << "0x" << std::hex << nextAddress;

		currentMemory[currentAddress] =
			"FFree Memory " + std::to_string(counter) + "\n" +
			addressStream.str() +
			"\nSize: " +
			std::to_string(mb) + "MB " +
			std::to_string(kb) + "kB " +
			std::to_string(bytes) + "bytes";

		pCurrentFreeEntry = pCurrentFreeEntry->pNext;
		counter++;
	} while (pCurrentFreeEntry != pStartFreeEntry);

	if (ImGui::TreeNode("Allocations"))
	{
		for (auto it : currentMemory)
		{
			std::string entry = it.second;
			char type = entry.substr(0, 1).c_str()[0];
			std::string entryTag = entry.substr(1, entry.length() - 1) + "\n\n";

			switch (type)
			{
			case 'A':
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), entryTag.c_str());
				break;
			case 'F':
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), entryTag.c_str());
				break;
			}
		}

		ImGui::TreePop();
	}
}

int main(int argc, const char* argv[])
{    
#if defined(_WIN32)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	//Start program
	sf::Color bgColor = sf::Color::Black;
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Game Engine Architecture");

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	std::cout << "Hello World" << std::endl;

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
			else if (event.type == sf::Event::KeyPressed)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				{
					window.close();
				}
			}
        }

        ImGui::SFML::Update(window, deltaClock.restart());

		ImGuiPrintMemoryManagerAllocations();

		window.clear(bgColor);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0; 
}
