#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
//#include <crtdbg.h>

#include "StackAllocator.h"

void testStackAllocator(unsigned int nrOfObjects)
{
	size_t size = nrOfObjects * sizeof(sf::CircleShape);
	void* start = alloca(size);
	void* end = (char*)start + size;
	StackAllocator<sf::CircleShape> allocator(start, end);
	sf::Clock timing;

	// ------------------------------------ test 1 -------------------------------

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		sf::CircleShape* tmp = new sf::CircleShape(100.f);
		delete tmp;
	}
	sf::Time t = timing.restart();

	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		sf::CircleShape* tmp = allocator.make_new(sf::CircleShape(100.f));
		allocator.make_delete(tmp);
	}
	sf::Time t2 = timing.restart();

	std::cout << "Test 1.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes) " <<  " on both the regular heap and implemented stack allocator." << std::endl << " Heap used " << t.asMilliseconds() << " milliseconds" << std::endl << 
		" Stack used " << t2.asMilliseconds() << " milliseconds" << std::endl;

}

int main(int argc, const char* argv[])
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Engine Architecture");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    
    std::cout << "Hello World" << std::endl;
    
    //sf::CircleShape shape(100.f);
    //shape.setFillColor(sf::Color::Green);

	testStackAllocator(1000);


	size_t size = sizeof(sf::CircleShape) * 100;
	void* start = alloca(size);
	void*end = (char*)start + size;

	StackAllocator<sf::CircleShape> allocator(start, end);

	

	sf::CircleShape* circle = allocator.make_new(sf::CircleShape(100.f));
	circle->setFillColor(sf::Color::Yellow);

	sf::CircleShape* second = allocator.make_new(sf::CircleShape(100.f));
	second->setFillColor(sf::Color::Red);
	second->setPosition(100, 0);

	allocator.make_delete(second);

	sf::CircleShape* third = allocator.make_new(sf::CircleShape(100.f));
	third->setFillColor(sf::Color::Blue);
	third->setPosition(0, 100);
	//second and third should have same adress now, so when second is used later, it will be of the new circle... (a blue one), UNSAFE! yes, but I wish to test it.

	third = allocator.make_new(sf::CircleShape(100.f));
	third->setFillColor(sf::Color::White);
	third->setPosition(100, 0);

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
		window.draw(*circle);
		window.draw(*second);
		window.draw(*third);
		ImGui::SFML::Render(window);
		window.display();
	}

	allocator.make_delete(circle);
	allocator.make_delete(second);
	allocator.make_delete(third);
	ImGui::SFML::Shutdown();
    return 0; 
}
