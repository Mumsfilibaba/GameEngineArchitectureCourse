#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "StackAllocator.h"
void testStackAllocator(unsigned int nrOfObjects, StackAllocator<sf::CircleShape>* allocator)
{
	sf::Clock timing;
	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		sf::CircleShape* tmp = new sf::CircleShape(100.f);
		delete tmp;
	}
	sf::Time t = timing.restart();

	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		sf::CircleShape* tmp = allocator->allocate();
		tmp = new(tmp) sf::CircleShape(100.f);
		allocator->free(tmp);
	}
	sf::Time t2 = timing.restart();

	std::cout << "Created and deleted " << nrOfObjects << " on both the regular heap and implemented stack allocator." << std::endl << "Heap used " << t.asMilliseconds() << " milliseconds" << std::endl << 
		"Stack used " << t2.asMilliseconds() << " milliseconds";
}

int main(int argc, const char* argv[])
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Engine Architecture");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    
    std::cout << "Hello World" << std::endl;
    
    //sf::CircleShape shape(100.f);
    //shape.setFillColor(sf::Color::Green);
	size_t size = sizeof(sf::CircleShape) * 100;
	void* start = alloca(size);
	void*end = (char*)start + size;

	StackAllocator<sf::CircleShape> allocator(start, end);

	testStackAllocator(10000, &allocator);

	sf::CircleShape* circle = allocator.allocate();
	circle = new(circle) sf::CircleShape(100.f);
	circle->setFillColor(sf::Color::Yellow);

	sf::CircleShape* second = allocator.allocate();
	second = new(second) sf::CircleShape(100.f);
	second->setFillColor(sf::Color::Red);
	second->setPosition(100, 0);

	allocator.free(second);
	sf::CircleShape* third = allocator.allocate();
	third = new(second) sf::CircleShape(100.f);
	third->setFillColor(sf::Color::Blue);
	third->setPosition(0, 100);
	//second and third should have same adress now, so when second is used later, it will be of the new circle...

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
        
		ImGui::ShowTestWindow();
        
		ImGui::Begin("Hello, world!");
		ImGui::Button("Look at this pretty button");
		ImGui::End();

		window.clear();
		//window.draw(shape);
		window.draw(*circle);
		window.draw(*second);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
    return 0; 
}
