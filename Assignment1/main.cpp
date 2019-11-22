#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <sstream>
#include <thread>
#include <array>
#include "PoolAllocator.h"
#if defined(_WIN32)
    #include <crtdbg.h>
#endif
#include "StackAllocator.h"

#define PI 3.14159265359f
#define MB(mb) mb * 1024 * 1024

#if defined(_WIN32)
	#define MEMLEAKCHECK _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
	#define MEMLEAKCHECK
#endif

#define NUMBER_OF_OBJECTS_IN_TEST 1024 * 64

#ifdef USE_CUSTOM_ALLOCATOR
#define STACK_NEW stack_new
#define STACK_DELETE(object) stack_delete(object)
#define POOL_NEW(type) pool_new(type)
#define POOL_DELETE(object) pool_delete(object)
#else
#define STACK_NEW new
#define STACK_DELETE delete
#define POOL_NEW(Type) new
#define POOL_DELETE(Object) delete Object
#endif

#ifdef TEST_POOL_ALLOCATOR
#define CHANCE_OF_ALLOCATION 0.3f
#define CHANCE_OF_FREE 0.3f
#endif

#ifdef MULTI_THREADED
	#define NUMBER_OF_THREADS_IN_MULTI_THREADED 4
	std::thread* gThreads[NUMBER_OF_THREADS_IN_MULTI_THREADED];
	bool g_ThreadsStarted = false;
	bool g_StopThreads = false;
	std::atomic_bool g_RunFrame = false;

	struct ThreadPerformanceData
	{
		std::string threadID;
		int	FPS = 0;
		int CurrentValue = 0;
		float AverageDelta = 0.0f;
		float Deltas[90] = {};
	};

	SpinLock g_ThreadPerfDataLock;
	std::unordered_map<std::thread::id, ThreadPerformanceData> g_ThreadPerfData;

	void InitThread()
	{
		std::lock_guard<SpinLock> lock(g_ThreadPerfDataLock);
		ThreadPerformanceData& data = g_ThreadPerfData[std::this_thread::get_id()];

		std::stringstream ss;
		ss << std::this_thread::get_id();
		data.threadID = ss.str();
	}


	void MeasureThreadPerf()
	{
		thread_local static int timer = 0;
		thread_local static int fps = 0;
		thread_local static int currentFps = 0;
		thread_local static sf::Clock deltaClock;

		{
			//Wait for mainthread to begin frame
			while (!g_RunFrame && !g_StopThreads);

			//Get deltatimes and fps
			sf::Time dt = deltaClock.restart();
			timer += dt.asMicroseconds();

			{
				std::lock_guard<SpinLock> lock(g_ThreadPerfDataLock);
				ThreadPerformanceData& data = g_ThreadPerfData[std::this_thread::get_id()];

				currentFps++;
				if (timer >= 1000000)
				{
					fps = currentFps;
					currentFps = 0;
					timer = 0;

					float avg = 0.0f;
					for (int i = 0; i < 90; i++)
						avg = avg + data.Deltas[i];
					avg = avg / 90.0f;

					data.AverageDelta = avg;
				}

				data.FPS = fps;
				data.Deltas[data.CurrentValue] = float(dt.asMicroseconds()) / 1000.0f;
				data.CurrentValue = (data.CurrentValue + 1) % 90;
			}
		}
	}
#else
	#define NUMBER_OF_THREADS_IN_MULTI_THREADED 1
#endif

#define NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD NUMBER_OF_OBJECTS_IN_TEST / NUMBER_OF_THREADS_IN_MULTI_THREADED

struct DummyStruct
{
	DummyStruct(float x, float y, float z, float rotationX, float rotationY, float rotationZ)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->rotationX = rotationX;
		this->rotationY = rotationY;
		this->rotationZ = rotationZ;
	}

	float x;
	float y;
	float z;
	float rotationX;
	float rotationY;
	float rotationZ;
};

thread_local static std::array<DummyStruct*, NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD> gContainerArr;

float randf()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float randf(float min, float max)
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max - min) + min;
}

void ThreadSafePrintf(const char* pFormat, ...)
{
	static SpinLock printLock;
	std::lock_guard<SpinLock> lock(printLock);

	va_list args;
	va_start(args, pFormat);
	vprintf(pFormat, args);
	va_end(args);
}

void ImGuiDrawMemoryProgressBar(int used, int available)
{
	float usedF = float(used) / float(std::max(available, 1));
	ImGui::ProgressBar(usedF, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("(%d/%d) Bytes", used, available);
}

void ImGuiPrintMemoryManagerAllocations()
{
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
}

void ImGuiDrawFrameTimeGraph(const sf::Time& dt)
{
	static int timer = dt.asMicroseconds();
	timer += dt.asMicroseconds();

	static int fps = 0;
	static int currentFps = 0;
	currentFps++;
	if (timer >= 1000000)
	{
		fps			= currentFps;
		currentFps	= 0;
		timer		= 0;
	}

	ImGui::SetNextWindowBgAlpha(0.75f); // Transparent background
	ImGui::Text("Frametime:");
	{
		constexpr int valueCount = 90;
		static float cpuValues[valueCount] = { 0 };
		static int   valuesOffset = 0;
		static float average = 0.0f;
		cpuValues[valuesOffset] = float(dt.asMicroseconds()) / 1000.0f;
		valuesOffset = (valuesOffset + 1) % valueCount;

		//Calc average
		if (timer == 0 && fps > 0)
		{
			average = 0;
			for (int i = 0; i < valueCount; i++)
				average += cpuValues[valuesOffset];
			average /= fps;
		}

		ImGui::Text("FPS: %d", fps);
		ImGui::Text("CPU Frametime (ms):");

		char overlay[32];
		sprintf(overlay, "Avg %f", average);
		ImGui::PlotLines("", cpuValues, valueCount, valuesOffset, overlay, 0.0f, 30.0f, ImVec2(0, 80));
	}

#ifdef MULTI_THREADED
	{
		constexpr int valueCount = 90;

		std::lock_guard<SpinLock> lock(g_ThreadPerfDataLock);
		for (auto dt : g_ThreadPerfData)
		{
			ThreadPerformanceData& data = dt.second;

			ImGui::Text("Frametime [Tread %s]:", data.threadID.c_str());
			{
				ImGui::Text("FPS: %d", data.FPS);
				ImGui::Text("CPU Frametime (ms):");

				char overlay[32];
				sprintf(overlay, "Avg %f", data.AverageDelta);

				ImGui::PlotLines("", data.Deltas, 90, data.CurrentValue, overlay, 0.0f, 30.0f, ImVec2(0, 80));
			}
		}
	}
#endif
}
#ifdef TEST_STACK_ALLOCATOR
void RunTest()
{
#ifdef MULTI_THREADED
	InitThread();

	while (!g_StopThreads)
	{
		MeasureThreadPerf();
#endif
		
		//Allocate a bunch of objects
		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
			gContainerArr[i] = STACK_NEW DummyStruct(
				randf(-1024.0f, 1024.0f),
				randf(-1024.0f, 1024.0f),
				randf(-1024.0f, 1024.0f),
				randf(0, 2 * PI),
				randf(0, 2 * PI),
				randf(0, 2 * PI));
		}

		//Do some calculations on those objects
		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
			int randomIndex = rand() % NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD;
			DummyStruct* a = gContainerArr[i];
			DummyStruct* b = gContainerArr[randomIndex];

			a->x += b->y;
			a->y += b->x * b->z;
			a->rotationY = cosf(b->x) * sinf(b->z) + cosf(a->x) * sinf(a->z);
			a->rotationY = cosf(b->x) * sinf(b->z) * sinf(a->y);
		}

		//Call object destructors
		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
			//Delete just to call destructors on object, actual memory is reused
			STACK_DELETE(gContainerArr[i]);
			gContainerArr[i] = nullptr;
		}

#ifdef MULTI_THREADED
#ifdef USE_CUSTOM_ALLOCATOR
		stack_reset();
#endif
	}

	for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
	{
		if (gContainerArr[i] != nullptr)
		{
			STACK_DELETE(gContainerArr[i]);
			gContainerArr[i] = nullptr;
		}
	}
#endif
}
#elif defined(TEST_POOL_ALLOCATOR)
void RunTest()
{
#ifdef MULTI_THREADED
	InitThread();

	while (!g_StopThreads)
	{
		MeasureThreadPerf();
#endif
		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
			if (gContainerArr[i] == nullptr)
			{
				if (randf() < CHANCE_OF_ALLOCATION)
				{
					gContainerArr[i] = POOL_NEW(DummyStruct) DummyStruct(
						randf(-1024.0f, 1024.0f),
						randf(-1024.0f, 1024.0f),
						randf(-1024.0f, 1024.0f),
						randf(0, 2 * PI),
						randf(0, 2 * PI),
						randf(0, 2 * PI));
				}
			}
		}

		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
			int randomIndex = rand() % NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD;
			DummyStruct* a = gContainerArr[i];
			DummyStruct* b = gContainerArr[randomIndex];

			if (a != nullptr && b != nullptr)
			{
				a->x += b->y;
				a->y += b->x * b->z;
				a->rotationY = cosf(b->x) * sinf(b->z) + cosf(a->x) * sinf(a->z);
				a->rotationY = cosf(b->x) * sinf(b->z) * sinf(a->y);
			}
		}

		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
			if (gContainerArr[i] != nullptr)
			{
				if (randf() < CHANCE_OF_FREE)
				{
					POOL_DELETE(gContainerArr[i]);
					gContainerArr[i] = nullptr;
				}
			}
		}
#ifdef MULTI_THREADED
	}

	for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
	{
		if (gContainerArr[i] != nullptr)
		{
			POOL_DELETE(gContainerArr[i]);
			gContainerArr[i] = nullptr;
		}
	}
#endif
}
#endif

#if defined(TEST_STACK_ALLOCATOR) || defined(TEST_POOL_ALLOCATOR)
#ifndef MULTI_THREADED
void StartTest()
{
	//Not Multi-threaded Stack Allocation Test
	RunTest();
}
#else
void StartTest()
{
	//Multi-threaded Stack Allocation Test
	if (!g_ThreadsStarted)
	{
		g_ThreadsStarted = true;

		for (int i = 0; i < NUMBER_OF_THREADS_IN_MULTI_THREADED; i++)
		{
			gThreads[i] = new std::thread(RunTest);
		}
	}
}
#endif
#endif

#ifndef MULTI_THREADED
#ifdef TEST_STACK_ALLOCATOR
void StopTest()
{
	//Not Multi-threaded Stack Allocation Test
	for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
	{
		if (gContainerArr[i] != nullptr)
		{
			STACK_DELETE(gContainerArr[i]);
			gContainerArr[i] = nullptr;
		}
	}
}
#elif defined(TEST_POOL_ALLOCATOR)
void StopTest()
{
	//Not Multi-threaded Stack Allocation Test
	for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
	{
		if (gContainerArr[i] != nullptr)
		{
			POOL_DELETE(gContainerArr[i]);
			gContainerArr[i] = nullptr;
		}
	}
}
#endif
#else
void StopTest()
{
	g_StopThreads = true;

	//Multi-threaded Stack Allocation Test
	for (int i = 0; i < NUMBER_OF_THREADS_IN_MULTI_THREADED; i++)
	{
		gThreads[i]->join();
		delete gThreads[i];
	}
}
#endif

#ifdef MULTI_THREADED
void BeginFrame()
{
	g_RunFrame = true;
}

void EndFrame()
{
	g_RunFrame = false;
}
#else
	#define BeginFrame()
	#define EndFrame()
#endif


int main(int argc, const char* argv[])
{    
	MEMLEAKCHECK;

	//Start program
	sf::Color bgColor = sf::Color::Black;
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Game Engine Architecture");
	window.setVerticalSyncEnabled(false);
	window.setFramerateLimit(0);
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

		sf::Time deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);

		ImGui::ShowDemoWindow();

#if defined(TEST_STACK_ALLOCATOR) || defined(TEST_POOL_ALLOCATOR)
		StartTest();
#endif
		BeginFrame();

		//Draw debug window
		if (ImGui::Begin("Debug Window"))
		{
			static bool v_borders = true;
			ImGui::Columns(2, "Memory", v_borders);
			{
				ImGuiDrawMemoryProgressBar(PoolAllocatorBase::GetTotalUsedMemory(), PoolAllocatorBase::GetTotalAvailableMemory());
				ImGui::NextColumn();
				ImGui::Text("Pool allocated memory");
				
				ImGui::NextColumn();
				ImGuiDrawMemoryProgressBar(StackAllocator::GetTotalUsedMemory(), StackAllocator::GetTotalAvailableMemory());
				ImGui::NextColumn();
				ImGui::Text("Stack allocated memory");

				ImGui::NextColumn();
				ImGuiDrawMemoryProgressBar(MemoryManager::GetTotalUsedMemory(), MemoryManager::GetTotalAvailableMemory());
				ImGui::NextColumn();
				ImGui::Text("Globaly allocated memory");
			}

			ImGui::Columns(1);
			ImGui::Separator();

			ImGui::Columns(2, "Memory", v_borders);
			ImGuiPrintMemoryManagerAllocations();
			
			ImGui::NextColumn();
			ImGuiDrawFrameTimeGraph(deltaTime);
		}
		ImGui::End();

		window.clear(bgColor);
		ImGui::SFML::Render(window);
		
		EndFrame();
		window.display();

#ifndef MULTI_THREADED
	#ifdef USE_CUSTOM_ALLOCATOR
		stack_reset();
	#endif
#endif
	}

#if defined(TEST_STACK_ALLOCATOR) || defined(TEST_POOL_ALLOCATOR)
	StopTest();
#endif

	ImGui::SFML::Shutdown();

	return 0; 
}
