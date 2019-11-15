#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <sstream>
#include "PoolAllocator.h"

PoolAllocator<int> g_Allocator(MB(4));

void ThreadSafePrintf(const char* pFormat, ...)
{
	static std::mutex printMutex;
	std::lock_guard<std::mutex> lock(printMutex);

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

	constexpr int count = 1024;
	ThreadSafePrintf("Total memory consumption: %d bytes\n", count * sizeof(void*));
	int** ppPoolAllocated = new int* [count];
	int** ppOSAllocated = new int* [count];

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

	ThreadSafePrintf("Freeing %d vars from pool took %d qs [THREAD %s]\n", count, (t2 - t1).asMicroseconds(), ss.str().c_str());

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
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Engine Architecture");
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
