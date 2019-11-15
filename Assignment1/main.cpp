#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <sstream>
#include "PoolAllocator.h"
#include <crtdbg.h>
#include "FrameAllocator.h"

#define MB(mb) mb * 1024 * 1024

int main(int argc, const char* argv[]);
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

template <class T, typename ... Args>
void testFrameAllocator(unsigned int nrOfObjects, Args&&... args)
{
	size_t size = (nrOfObjects) * sizeof(T);
	//FrameAllocator allocator(size);
	FrameAllocator& allocator = FrameAllocator::getInstance();
	sf::Clock timing;

	// ------------------------------------ test 1 -------------------------------

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		T* tmp = new T(std::forward<Args>(args)...);
		delete tmp;
	}
	sf::Time t = timing.restart();

	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		T* tmp = allocator.allocate<T>(std::forward<Args>(args)...);
		tmp->~T();
		allocator.reset();
	}
	sf::Time t2 = timing.restart();

	std::cout << "Test 1.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes)" << " on both the regular heap and implemented stack allocator." << std::endl << " Heap used " << t.asMilliseconds() << " milliseconds" << std::endl <<
		" Stack used " << t2.asMilliseconds() << " milliseconds" << std::endl;

	//create pointers so that we can keep track of object deletion later.
	T** pTmp = new T*[nrOfObjects];

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i] = new T(std::forward<Args>(args)...);
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
		pTmp[i] = allocator.allocate<T>(std::forward<Args>(args)...);
	}

	t = timing.restart();

	// Whenever objects need to be destroyed...
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i]->~T();
	}

	allocator.reset();

	t2 = timing.restart();

	std::cout << " Created and deleted " << nrOfObjects << " objects(" << size << " bytes) " << " on the implemented stack.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl;

	delete[] pTmp;
}

int main(int argc, const char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

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

	//testStackAllocator(10);
	testFrameAllocator<int>(10000000, 100);

	FrameAllocator& frameAllocator = FrameAllocator::getInstance();
	frameAllocator.allocate<int>(5);
	frameAllocator.allocate<char>('c');
	int* arr = frameAllocator.allocateArray<int>(3);
	for (int i = 0; i < 3; i++)
	{
		arr[i] = i;
	}

	frameAllocator.reset();

	sf::CircleShape* magenta = frameAllocator.allocate<sf::CircleShape>(100.f);

	magenta->setFillColor(sf::Color::Magenta);
	magenta->setPosition(100, 100);

	sf::CircleShape* green = frameAllocator.allocate<sf::CircleShape>(100.f);
	green->setFillColor(sf::Color::Green);

	sf::CircleShape* red = frameAllocator.allocate<sf::CircleShape>(100.f);

	red->setFillColor(sf::Color::Red);
	red->setPosition(0, 100);

	sf::CircleShape* blue = frameAllocator.allocate<sf::CircleShape>(100.f);

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

	frameAllocator.reset();

    return 0; 
}
