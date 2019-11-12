#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <crtdbg.h>

#include "StackAllocator.h"
#include "FrameAllocator.h"

void testStackAllocator(unsigned int nrOfObjects)
{
	size_t size = nrOfObjects * sizeof(int);
	StackAllocator allocator(size);
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
		int* tmp = (int*)allocator.make_new(5);
		allocator.make_delete(tmp);
	}
	sf::Time t2 = timing.restart();

	std::cout << "Test 1.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes)" <<  " on both the regular heap and implemented stack allocator." << std::endl << " Heap used " << t.asMilliseconds() << " milliseconds" << std::endl << 
		" Stack used " << t2.asMilliseconds() << " milliseconds" << std::endl;

	//create pointers so that we can keep track of object deletion later.
	int** pTmp = new int*[nrOfObjects];

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i] = new int(5);
	}
	t = timing.restart();

	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		delete pTmp[i];
	}
	t2 = timing.restart();
	
	std::cout << "Test 2.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes) " << " on the regular heap.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl << std::endl;

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i] = (int*)allocator.make_new(5);
	}
	t = timing.restart();

	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		allocator.make_delete(pTmp[i]);
	}
	t2 = timing.restart();

	std::cout << " Created and deleted " << nrOfObjects << " objects(" << size << " bytes) " << " on the implemented stack.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl;

	delete[] pTmp;
}

void testFrameAllocator(unsigned int nrOfObjects)
{
	size_t size = nrOfObjects * sizeof(int);
	FrameAllocator allocator(size);
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
		sf::CircleShape* tmp = (sf::CircleShape*)allocator.allocate(sf::CircleShape(100.f));
		allocator.reset();
	}
	sf::Time t2 = timing.restart();

	std::cout << "Test 1.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes)" << " on both the regular heap and implemented stack allocator." << std::endl << " Heap used " << t.asMilliseconds() << " milliseconds" << std::endl <<
		" Stack used " << t2.asMilliseconds() << " milliseconds" << std::endl;

	//create pointers so that we can keep track of object deletion later.
	sf::CircleShape** pTmp = new sf::CircleShape*[nrOfObjects];

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i] = new sf::CircleShape(100.f);
	}
	t = timing.restart();

	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		delete pTmp[i];
	}
	t2 = timing.restart();

	std::cout << "Test 2.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes) " << " on the regular heap.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl << std::endl;

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i] = (sf::CircleShape*)allocator.allocate(sf::CircleShape(100.f));
	}
	t = timing.restart();

	allocator.reset();

	t2 = timing.restart();

	std::cout << " Created and deleted " << nrOfObjects << " objects(" << size << " bytes) " << " on the implemented stack.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl;

	delete[] pTmp;
}

int main(int argc, const char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Engine Architecture");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    
    std::cout << "Hello World" << std::endl;

	//testStackAllocator(10);
	//testFrameAllocator(1000000);

	size_t size = sizeof(sf::CircleShape) * 10;
	FrameAllocator fAlloc(size);
	int* tst = (int*)fAlloc.allocate(int(5));
	char* tstChar = fAlloc.allocate(char('c'));
	int* arr = fAlloc.allocateArray<int>(3);
	for (int i = 0; i < 3; i++)
	{
		arr[i] = i;
	}

	fAlloc.reset();

	sf::CircleShape* magenta = fAlloc.allocate(sf::CircleShape(100.f));

	magenta->setFillColor(sf::Color::Magenta);
	magenta->setPosition(100, 100);

	sf::CircleShape* green = fAlloc.allocate(sf::CircleShape(100.f));
	green->setFillColor(sf::Color::Green);

	sf::CircleShape* red = fAlloc.allocate(sf::CircleShape(100.f));

	red->setFillColor(sf::Color::Red);
	red->setPosition(0, 100);

	sf::CircleShape* blue = fAlloc.allocate(sf::CircleShape(100.f));

	blue->setFillColor(sf::Color::Blue);
	blue->setPosition(100, 0);

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
		window.draw(*green);
		window.draw(*blue);
		window.draw(*red);
		window.draw(*magenta);
		ImGui::SFML::Render(window);
		window.display();
	}
	ImGui::SFML::Shutdown();

	// as long as destructor is called for these (before we lose them in memory) a destruction has to be called
	magenta->~CircleShape();
	green->~CircleShape();
	blue->~CircleShape();
	red->~CircleShape();

	fAlloc.reset();

    return 0; 
}
