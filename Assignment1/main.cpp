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


float heapDelCrea;
float stackDelCrea;
float heapCrea;
float stackCrea;
float heapDel;
float stackDel;


template <class T, typename ... Args>
void testFrameAllocator(unsigned int nrOfObjects, Args&&... args)
{
	size_t size = (nrOfObjects) * sizeof(T);
	FrameAllocator allocator(size);
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

	heapDelCrea += t.asMilliseconds();
	stackDelCrea += t2.asMilliseconds();
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

	heapCrea += t.asMilliseconds();
	heapDel += t2.asMilliseconds();
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
	stackCrea += t.asMilliseconds();
	stackDel += t2.asMilliseconds();
	std::cout << " Created and deleted " << nrOfObjects << " objects(" << size << " bytes) " << " on the implemented stack.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl;
	
	delete[] pTmp;
}

int main(int argc, const char* argv[])
{
	float totalTimeList[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	int runCount = 0;
	int nrOfObjects = 0;
	int nrOfArgs = 0;
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Engine Architecture");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    
    std::cout << "Hello World" << std::endl;

	
	//testFrameAllocator<int>(10000000, 100);
	bool runTest = false;
	size_t size = sizeof(sf::CircleShape) * 10;
	FrameAllocator fAlloc(size);
	fAlloc.allocate<int>(5);
	fAlloc.allocate<char>('c');
	int* arr = fAlloc.allocateArray<int>(3);
	for (int i = 0; i < 3; i++)
	{
		arr[i] = i;
	}

	sf::Color bgColor;
	char windowTitle[255] = "ImGui + SFML = <3";
	float color[3] = { 0.f, 0.f, 0.f };
	fAlloc.reset();

	sf::CircleShape* magenta = fAlloc.allocate<sf::CircleShape>(100.f);

	magenta->setFillColor(sf::Color::Magenta);
	magenta->setPosition(100, 100);

	sf::CircleShape* green = fAlloc.allocate<sf::CircleShape>(100.f);
	green->setFillColor(sf::Color::Green);

	sf::CircleShape* red = fAlloc.allocate<sf::CircleShape>(100.f);

	red->setFillColor(sf::Color::Red);
	red->setPosition(0, 100);

	sf::CircleShape* blue = fAlloc.allocate<sf::CircleShape>(100.f);

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

		ImGui::Begin("Hello, world!");
		ImGui::InputInt("Number of Test Runs: ", &runCount);
		ImGui::InputInt("Number Of Objects to allocate: ", &nrOfObjects);
		ImGui::InputInt("Number Of args?: ", &nrOfArgs);

		if (ImGui::Button("GO!"))
		{
			runTest = true;

			for (int i = 0; i < runCount; i++)
			{
				testFrameAllocator<int>(nrOfObjects, nrOfArgs);
			}

			heapDelCrea = heapDelCrea / runCount;
			stackDelCrea = stackDelCrea / runCount;
			heapCrea = heapCrea / runCount;
			stackCrea = stackCrea / runCount;
			heapDel = heapDel / runCount;
			stackDel = stackDel / runCount;
			totalTimeList[0] = heapDelCrea;
			totalTimeList[1] = stackDelCrea;
			totalTimeList[2] = heapDel;
			totalTimeList[3] = heapCrea;
			totalTimeList[4] = stackDel;
			totalTimeList[5] = stackCrea;
		}

	

		std::string text = "Heap time deletion & creation: \n" + std::to_string(heapDelCrea) + " milliseconds";
		std::string text1 = "Stack time deletion & creation: \n" + std::to_string(stackDelCrea) + " milliseconds";
		std::string text2 = "Heap time deletion: \n" + std::to_string(heapDel) + " milliseconds";
		std::string text3 = "Heaptime creation: \n" + std::to_string(heapCrea) + " milliseconds";
		std::string text4 = "Stack time deletion : \n" + std::to_string(stackDel) + " milliseconds";
		std::string text5 = "Stack time creation: \n" + std::to_string(stackCrea) + " milliseconds";
		if (runTest)
		{	
			ImGui::Text("Total time for creation and deletion: ");
			ImGui::BulletText(text.c_str());
			ImGui::BulletText(text1.c_str());
			ImGui::Text("Total time for creation and deletion on regular Heap: ");
			ImGui::BulletText(text2.c_str());
			ImGui::BulletText(text3.c_str());
			ImGui::Text("Total time for creation and deletion on Implemented Stack Allocator: ");
			ImGui::BulletText(text4.c_str());
			ImGui::BulletText(text5.c_str());
		}

		ImGui::PlotHistogram(
			"Average allocation time Table",
			totalTimeList,
			IM_ARRAYSIZE(totalTimeList),
			0, 
			"1. Heap Create & delete, 2. Stack Create & delete \n3. Heap delete, 4. Heap Create \n5. Stack Delete, 6. Stack Create",
			0.0f,
			2000.0f,
			ImVec2(400, 200));

		ImGui::ShowTestWindow();


		
		ImGui::Separator();
		// Background color edit
		if (ImGui::ColorEdit3("Background color", color)) {
			bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
			bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
			bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
		}
		ImGui::Separator();
		// Window title text edit
		ImGui::InputText("Window title", windowTitle, 255);
		ImGui::Separator();
		if (ImGui::Button("Update window title")) {
			window.setTitle(windowTitle);
		}

		ImGui::End(); // end window


		


		window.clear(bgColor);
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
