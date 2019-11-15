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
