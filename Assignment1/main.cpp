#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <sstream>
#include "PoolAllocator.h"

#define MB(mb) mb * 1024 * 1024
PoolAllocator<int> g_Allocator;

void ThreadSafePrintf(const char* pFormat, ...)
{
	static SpinLock printLock;
	std::lock_guard<SpinLock> lock(printLock);

	va_list args;
	va_start(args, pFormat);
	vprintf(pFormat, args);
	va_end(args);
}

void Func()
{
	sf::Clock clock;
	
	std::stringstream ss;
	ss << std::this_thread::get_id();

	constexpr int count = 4096;
	ThreadSafePrintf("Total memory consumption: %d bytes [THREAD %s]\n", count * sizeof(void*), ss.str().c_str());
	int** ppPoolAllocated = new int*[count];
	int** ppOSAllocated = new int*[count];

	//Allocate from pool
	clock.restart();
	sf::Time t1 = clock.getElapsedTime();
	for (int i = 0; i < count; i++)
	{
		ppPoolAllocated[i] = g_Allocator.MakeNew(i);
	}
	sf::Time t2 = clock.getElapsedTime();

	ThreadSafePrintf("Allocating %d vars from pool took %d qs [THREAD %s]\n", count, (t2 - t1).asMicroseconds(), ss.str().c_str());

	//Allocate from OS
	clock.restart();
	t1 = clock.getElapsedTime();
	for (int i = 0; i < count; i++)
	{
		ppOSAllocated[i] = new int(i);
	}
	t2 = clock.getElapsedTime();

	ThreadSafePrintf("Allocating %d vars from OS took %d qs [THREAD %s]\n", count, (t2 - t1).asMicroseconds(), ss.str().c_str());

	//Free to pool
	clock.restart();
	t1 = clock.getElapsedTime();
	for (int i = 0; i < count; i++)
	{
		g_Allocator.Free(ppPoolAllocated[i]);
		ppPoolAllocated[i] = nullptr;
	}
	t2 = clock.getElapsedTime();

	ThreadSafePrintf("Freeing %d vars from pool took %d qs [THREAD %s]\n", count, (t2 - t1).asMicroseconds(), ss.str().c_str());

	//Free to OS
	clock.restart();
	t1 = clock.getElapsedTime();
	for (int i = 0; i < count; i++)
	{
		delete ppOSAllocated[i];
		ppOSAllocated[i] = nullptr;
	}
	t2 = clock.getElapsedTime();

	ThreadSafePrintf("Freeing %d vars from OS took %d qs [THREAD %s]\n", count, (t2 - t1).asMicroseconds(), ss.str().c_str());

	delete ppPoolAllocated;
	ppPoolAllocated = nullptr;

	delete ppOSAllocated;
	ppOSAllocated = nullptr;
}


int main(int argc, const char* argv[])
{
	std::thread t1(Func);
	std::thread t2(Func);
	std::thread t3(Func);
	std::thread t4(Func);

	if (t1.joinable())
		t1.join();

	if (t2.joinable())
		t2.join();

	if (t3.joinable())
		t3.join();

	if (t4.joinable())
		t4.join();


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
