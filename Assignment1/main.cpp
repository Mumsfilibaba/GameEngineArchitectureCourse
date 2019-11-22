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

void ThreadSafePrintf(const char* pFormat, ...)
{
	static SpinLock printLock;
	std::lock_guard<SpinLock> lock(printLock);

	va_list args;
	va_start(args, pFormat);
	vprintf(pFormat, args);
	va_end(args);
}
//global pool allocator vars..
float g_totalMemoryConsumption = 0;
float g_availableMemory = PoolAllocator<int>::Get().GetTotalMemory();

void Func()
{
	sf::Clock clock;
	
	std::stringstream ss;
	ss << std::this_thread::get_id();

	constexpr int count = 4096;
	//g_totalMemoryConsumption = count * sizeof(void*);
	ThreadSafePrintf("Total memory consumption: %d bytes [THREAD %s]\n", count * sizeof(void*), ss.str().c_str());
	int** ppPoolAllocated	= (int**)allocate(sizeof(int*)*count, 1, "TestVars");
	int** ppOSAllocated		= new int*[count];

	//Allocate from pool
	clock.restart();
	sf::Time t1 = clock.getElapsedTime();
	for (int i = 0; i < count; i++)
	{
		ppPoolAllocated[i] = pool_new(int) int(i);
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
		pool_delete(ppPoolAllocated[i]);
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

	delete ppOSAllocated;
	ppOSAllocated = nullptr;
}

//global frame allocator vars..
float g_heapDelCrea;
float g_stackDelCrea;
float g_heapCrea;
float g_stackCrea;
float g_heapDel;
float g_stackDel;


template <class T, typename ... Args>
void testFrameAllocator(unsigned int nrOfObjects, Args&&... args)
{
	size_t size = (nrOfObjects) * sizeof(T);
	//FrameAllocator allocator(size);
	StackAllocator& allocator = StackAllocator::GetInstance();
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
		T* tmp = allocator.Allocate<T>(std::forward<Args>(args)...);
		tmp->~T();
		allocator.Reset();
	}
	sf::Time t2 = timing.restart();

	g_heapDelCrea += t.asMilliseconds();
	g_stackDelCrea += t2.asMilliseconds();
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

	g_heapCrea += t.asMilliseconds();
	g_heapDel += t2.asMilliseconds();
	std::cout << "Test 2.\n Created and deleted " << nrOfObjects << " objects (" << size << " bytes) " << " on the regular heap.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl << std::endl;

	timing.restart();
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i] = allocator.Allocate<T>(std::forward<Args>(args)...);
	}

	t = timing.restart();

	// Whenever objects need to be destroyed...
	for (unsigned int i = 0; i < nrOfObjects; i++)
	{
		pTmp[i]->~T();
	}

	allocator.Reset();

	t2 = timing.restart();
	g_stackCrea += t.asMilliseconds();
	g_stackDel += t2.asMilliseconds();
	std::cout << " Created and deleted " << nrOfObjects << " objects(" << size << " bytes) " << " on the implemented stack.\n creation took " << t.asMilliseconds() << " milliseconds\n Deletion took " << t2.asMilliseconds() << " milliseconds" << std::endl;
	
	delete[] pTmp;
}

int main(int argc, const char* argv[])
{
	float totalTimeList[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	int runCount = 0;
	int nrOfObjects = 0;
	int nrOfArgs = 0;
    
#if defined(_WIN32)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    
	bool runOnce = true;

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

	//return 0;

	//testStackAllocator(10);
	//testFrameAllocator<int>(10000000, 100);

	StackAllocator& stackAllocator = StackAllocator::GetInstance();
	stackAllocator.Allocate<int>(5);
	stackAllocator.AllocateAligned<char>(16, 'c');
	int* arr = stackAllocator.AllocateArray<int>(3);
	
	//testFrameAllocator<int>(10000000, 100);
	bool runTest = false;
	for (int i = 0; i < 3; i++)
	{
		arr[i] = i;
	}

	stackAllocator.Reset();
	sf::Color bgColor;
	char windowTitle[255] = "ImGui + SFML = <3";
	float color[3] = { 0.f, 0.f, 0.f };

	sf::CircleShape* magenta = stackAllocator.Allocate<sf::CircleShape>(100.f);

	magenta->setFillColor(sf::Color::Magenta);
	magenta->setPosition(100, 100);

	sf::CircleShape* green = stackAllocator.Allocate<sf::CircleShape>(100.f);
	green->setFillColor(sf::Color::Green);

	sf::CircleShape* red = stackAllocator.Allocate<sf::CircleShape>(100.f);

	red->setFillColor(sf::Color::Red);
	red->setPosition(0, 100);

	sf::CircleShape* blue = stackAllocator.Allocate<sf::CircleShape>(100.f);

	blue->setFillColor(sf::Color::Blue);
	blue->setPosition(100, 0);

	for (int i = 0; i < 1024; i++)
	{
		size_t randomSizeExp = (rand() % 16) + 1;
		size_t randomAlignmentExp = (rand() % randomSizeExp);
		size_t randomSize = pow(2, randomSizeExp);
		size_t randomAlignment = pow(2, randomAlignmentExp);
		std::cout << "Test Allocation " << i << " Size: " << randomSize << " Alignment: " << randomAlignment << std::endl;
		MemoryManager::GetInstance().Allocate(randomSize, randomAlignment, "Test Allocation " + std::to_string(i));
	}

	//Start program
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
        
		if (ImGui::Button("Run Pool Allocator"))
		{
			if (runOnce)
			{
				//Fixa (detta JA!) ?
				//t1 = std::thread(Func);
				//runOnce = false;
				std::cout << PoolAllocator<int>::Get().GetTotalMemory() << std::endl;
			}

		}

		ImGui::ProgressBar(g_totalMemoryConsumption / (4096 * sizeof(void*)), ImVec2(0.0f, 0.0f));
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Total Memory Consumption");


		char buff[32];
		std::sprintf(buff, "%d/%d", (int)(g_totalMemoryConsumption), (int)g_availableMemory);
		ImGui::ProgressBar( (g_totalMemoryConsumption/g_availableMemory), ImVec2(0.0f, 0.0f), buff);
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Available Memory");

		ImGui::Separator();
		ImGui::ShowTestWindow();

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

		static const char* current_item = NULL;

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

		ImGui::Begin("Hello, world!");
		ImGui::InputInt("Number of Test Runs: ", &runCount);
		ImGui::InputInt("Number Of Objects to allocate: ", &nrOfObjects);
		ImGui::InputInt("Number Of args?: ", &nrOfArgs);

		if (ImGui::Button("Run Frame Allocator"))
		{
			runTest = true;

			for (int i = 0; i < runCount; i++)
			{
				testFrameAllocator<int>(nrOfObjects, nrOfArgs);
			}

			g_heapDelCrea = g_heapDelCrea / runCount;
			g_stackDelCrea = g_stackDelCrea / runCount;
			g_heapCrea = g_heapCrea / runCount;
			g_stackCrea = g_stackCrea / runCount;
			g_heapDel = g_heapDel / runCount;
			g_stackDel = g_stackDel / runCount;
			totalTimeList[0] = g_heapDelCrea;
			totalTimeList[1] = g_stackDelCrea;
			totalTimeList[2] = g_heapDel;
			totalTimeList[3] = g_heapCrea;
			totalTimeList[4] = g_stackDel;
			totalTimeList[5] = g_stackCrea;
		}

	

		std::string text = "Heap time deletion & creation: \n" + std::to_string(g_heapDelCrea) + " milliseconds";
		std::string text1 = "Stack time deletion & creation: \n" + std::to_string(g_stackDelCrea) + " milliseconds";
		std::string text2 = "Heap time deletion: \n" + std::to_string(g_heapDel) + " milliseconds";
		std::string text3 = "Heaptime creation: \n" + std::to_string(g_heapCrea) + " milliseconds";
		std::string text4 = "Stack time deletion : \n" + std::to_string(g_stackDel) + " milliseconds";
		std::string text5 = "Stack time creation: \n" + std::to_string(g_stackCrea) + " milliseconds";
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

	stackAllocator.Reset();

	if (t1.joinable())
		t1.join();

	if (t2.joinable())
		t2.join();

	if (t3.joinable())
		t3.join();

	if (t4.joinable())
		t4.join();

    return 0; 
}
