#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "PoolAllocator.h"

#define MB(mb) mb * 1024 * 1024

int main(int argc, const char* argv[])
{
	PoolAllocator<int> allocator(MB(2));
    
    sf::Clock clock;

    constexpr int count = 4096 * 64;
    std::cout << "Total memory consumption: " << count * sizeof(long long) << " bytes" << std::endl;
    int** ppPoolAllocated	= new int*[count];
    int** ppOSAllocated		= new int*[count];
    
    //Allocate from pool
    clock.restart();
    sf::Time t1 = clock.getElapsedTime();
    for (int i = 0; i < count; i++)
    {
        ppPoolAllocated[i] = allocator.MakeNew(i);
    }
    sf::Time t2 = clock.getElapsedTime();
    
    std::cout << "Allocating " << count << " vars from pool took: " << (t2-t1).asMicroseconds() << "qs" << std::endl;
    
    //Allocate from OS
    clock.restart();
    t1 = clock.getElapsedTime();
    for (int i = 0; i < count; i++)
    {
        ppOSAllocated[i] = new int(i);
    }
    t2 = clock.getElapsedTime();
    
    std::cout << "Allocating " << count << " vars from OS took: " << (t2-t1).asMicroseconds() << "qs" << std::endl;
    
    //Free to pool
    clock.restart();
    t1 = clock.getElapsedTime();
    for (int i = 0; i < count; i++)
    {
        allocator.Free(ppPoolAllocated[i]);
        ppPoolAllocated[i] = nullptr;
    }
    t2 = clock.getElapsedTime();
    
    std::cout << "Freeing " << count << " vars from pool took: " << (t2-t1).asMicroseconds() << "qs" << std::endl;
    
    //Free to OS
    clock.restart();
    t1 = clock.getElapsedTime();
    for (int i = 0; i < count; i++)
    {
        delete ppOSAllocated[i];
        ppOSAllocated[i] = nullptr;
    }
    t2 = clock.getElapsedTime();
    
    std::cout << "Freeing " << count << " vars from OS took: " << (t2-t1).asMicroseconds() << "qs" << std::endl;

	delete ppPoolAllocated;
	ppPoolAllocated = nullptr;

	delete ppOSAllocated;
	ppOSAllocated = nullptr;


	//Start program
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Game Engine Architecture");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    
    std::cout << "Hello World" << std::endl;
    
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

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
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        
		ImGui::ShowTestWindow();

		std::map<size_t, std::string> currentMemory = std::map<size_t, std::string>(MemoryManager::GetInstance().GetAllocations());
		const FreeEntry* pCurrentFreeEntry = MemoryManager::GetInstance().GetFreeList();

		size_t counter = 0;
		while (pCurrentFreeEntry != nullptr)
		{
			size_t currentAddress = (size_t)pCurrentFreeEntry;
			size_t nextAddress = (size_t)pCurrentFreeEntry->pNext;

			std::stringstream currentAddressStream;
			currentAddressStream << std::hex << currentAddress;

			std::stringstream nextAddressStream;
			nextAddressStream << std::hex << nextAddress;

			currentMemory[currentAddress] =
				"FFree Memory " + std::to_string(counter) + 
				"\nStart: " + currentAddressStream.str() +
				"\nSize: " + std::to_string(pCurrentFreeEntry->sizeInBytes / 1024) + "kB\nNext: " +
				nextAddressStream.str();

			pCurrentFreeEntry = pCurrentFreeEntry->pNext;
			counter++;
		}

		static const char* current_item = NULL;

		if (ImGui::TreeNode("Allocations"))
		{
			for (auto& it = currentMemory.begin(); it != currentMemory.end(); it++)
			{
				std::string entry = it->second;
				char type = entry.substr(0, 1).c_str()[0];
				std::string entryTag = entry.substr(1, entry.length() - 1);

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

		ImGui::Begin("Hello, world!");
		ImGui::Button("Look at this pretty button");
		ImGui::End();

		window.clear();
		window.draw(shape);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
    return 0; 
}
