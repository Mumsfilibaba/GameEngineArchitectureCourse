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
#include <algorithm>
#include <fstream>
#include "PoolAllocator.h"
#include "Defines.h"
#if defined(_WIN32)
    #include <crtdbg.h>
#endif
#include "StackAllocator.h"

#define PI 3.14159265359f
#define MB(mb) (float)mb * 1024.0f * 1024.0f
#define BTOMB(mb) (float)mb / (1024.0f * 1024.0f)

#if defined(_WIN32)
	#define MEMLEAKCHECK _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
	#define MEMLEAKCHECK
#endif

#define NUMBER_OF_OBJECTS_IN_TEST 1024 * 16

#ifdef USE_CUSTOM_ALLOCATOR
#ifdef SHOW_ALLOCATIONS_DEBUG
#define STACK_NEW(tag) stack_new(tag)
#define STACK_DELETE(object) stack_delete(object)
#define POOL_NEW(type, tag) pool_new(type, tag)
#define POOL_DELETE(object) pool_delete(object)
#else
#define STACK_NEW(tag) stack_new
#define STACK_DELETE(object) stack_delete(object)
#define POOL_NEW(type, tag) pool_new(type)
#define POOL_DELETE(object) pool_delete(object)
#endif
#else
#define STACK_NEW(tag) new
#define STACK_DELETE(object) delete object
#define POOL_NEW(type, tag) new
#define POOL_DELETE(object) delete object
#endif

#ifdef TEST_POOL_ALLOCATOR
#define CHANCE_OF_ALLOCATION 0.3f
#define CHANCE_OF_FREE 0.3f
#endif

#ifdef COLLECT_PERFORMANCE_DATA
#define NUM_FRAMES_TO_COLLECT_OVER 10000
	std::map<std::thread::id, size_t> g_ThreadFrameSums;
#endif

std::string N2HexStr(size_t w)
{
	static const char* digits = "0123456789abcdef";
	static const size_t hex_len = 16;
	std::string rc(hex_len + 2, '0');
	rc[0] = '0';
	rc[1] = 'x';
	for (size_t i = 2, j = (hex_len - 1) * 4; i < hex_len + 2; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}

#ifdef MULTI_THREADED
	#define NUMBER_OF_THREADS_IN_MULTI_THREADED 4
	std::thread* gThreads[NUMBER_OF_THREADS_IN_MULTI_THREADED];
	bool g_ThreadsStarted = false;
	bool g_StopThreads = false;
	std::atomic_bool g_RunFrame = false;

#ifdef SHOW_GRAPHS
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
#endif
#else
	#define NUMBER_OF_THREADS_IN_MULTI_THREADED 1
#endif
#ifdef COLLECT_PERFORMANCE_DATA
	SpinLock g_ThreadFrameSumsLock;

	struct ThreadPerformanceData
	{
		ThreadPerformanceData()
		{
			this->threadID = std::this_thread::get_id();
			this->frameTimeSum = 0.0f;
		}

		~ThreadPerformanceData()
		{
			std::lock_guard<SpinLock> lock(g_ThreadFrameSumsLock);
			g_ThreadFrameSums[threadID] = frameTimeSum;
		}

		std::thread::id threadID;
		size_t frameTimeSum;
	};

	thread_local static ThreadPerformanceData threadPerformanceData;

	void MeasureThreadPerf()
	{
		thread_local static sf::Clock deltaClock;

		{
			//Get deltatimes and fps
			sf::Time dt = deltaClock.restart();
			threadPerformanceData.frameTimeSum += dt.asMicroseconds();
		}
	}
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
	ImGui::Text("(%.2f/%.2f) mb", BTOMB(used), BTOMB(available));
}

#ifdef SHOW_ALLOCATIONS_DEBUG
void ImGuiPrintMemoryManagerAllocations()
{
	static bool showMemoryManagerAllocations = true;
	static bool showMemoryManagerFreeBlock = true;
	static bool showPoolAllocations = true;
	static bool showStackAllocations = true;

	ImGui::Checkbox("Memory Manager Allocations", &showMemoryManagerAllocations);
	ImGui::Checkbox("Memory Manager Free Block", &showMemoryManagerFreeBlock);
	ImGui::Checkbox("Pool Allocations", &showPoolAllocations);
	ImGui::Checkbox("Stack Allocations", &showStackAllocations);

	if (ImGui::TreeNode("Allocations"))
	{
		auto& memoryManagerAllocationsRef = MemoryManager::GetInstance().GetAllocations();
		auto& poolAllocationsRef = MemoryManager::GetInstance().GetPoolAllocations();
		auto& stackAllocationsRef = MemoryManager::GetInstance().GetStackAllocations();

		MemoryManager::GetInstance().GetMemoryLock().lock();
		auto memoryManagerAllocations = std::map<size_t, Allocation>(memoryManagerAllocationsRef);
		MemoryManager::GetInstance().GetMemoryLock().unlock();

		MemoryManager::GetInstance().GetPoolAllocationLock().lock();
		auto poolAllocations = std::map<size_t, SubAllocation>(poolAllocationsRef);
		MemoryManager::GetInstance().GetPoolAllocationLock().unlock();

		MemoryManager::GetInstance().GetStackAllocationLock().lock();
		auto stackAllocations = std::map<size_t, SubAllocation>(stackAllocationsRef);
		MemoryManager::GetInstance().GetStackAllocationLock().unlock();

		auto& memoryManagerAllocationIt = memoryManagerAllocations.begin();
		auto& poolAllocationIt = poolAllocations.begin();
		auto& stackAllocationIt = stackAllocations.begin();
		const void* pMemoryManagerStart = MemoryManager::GetInstance().GetMemoryStart();
		const FreeEntry* pStartFreeEntry = MemoryManager::GetInstance().GetFreeList();
		const FreeEntry* pCurrentFreeEntry = pStartFreeEntry;

		size_t lastAddress = 0;
		size_t currentAddress = (size_t)pMemoryManagerStart;

		while (true)
		{
			size_t distanceToMemoryManagerAllocation = ULLONG_MAX;
			if (memoryManagerAllocationIt != memoryManagerAllocations.end())
				distanceToMemoryManagerAllocation = memoryManagerAllocationIt->first - currentAddress;

			size_t distanceToPoolAllocation = ULLONG_MAX;
			if (poolAllocationIt != poolAllocations.end())
				distanceToPoolAllocation = poolAllocationIt->first - currentAddress;

			size_t distanceToStackAllocation = ULLONG_MAX;
			if (stackAllocationIt != stackAllocations.end())
				distanceToStackAllocation = stackAllocationIt->first - currentAddress;

			size_t distanceToFreeEntry = ULLONG_MAX;
			if (pCurrentFreeEntry != pStartFreeEntry || lastAddress == 0)
				distanceToFreeEntry = (size_t)pCurrentFreeEntry - currentAddress;

			size_t minDistance = std::min(distanceToFreeEntry, std::min(distanceToStackAllocation, std::min(distanceToMemoryManagerAllocation, distanceToPoolAllocation)));

			if (distanceToMemoryManagerAllocation == ULLONG_MAX &&
				distanceToPoolAllocation == ULLONG_MAX &&
				distanceToStackAllocation == ULLONG_MAX &&
				distanceToFreeEntry == ULLONG_MAX)
				break;

			if (minDistance == distanceToMemoryManagerAllocation)
			{
				std::string allocationStr =
					memoryManagerAllocationIt->second.tag + "\n"
					"Address:   " + N2HexStr(memoryManagerAllocationIt->first) + "\n"
					"Padding: " + std::to_string(memoryManagerAllocationIt->second.padding) + "\n"
					"Size: " + std::to_string(memoryManagerAllocationIt->second.sizeInBytes) + "bytes\n\n";
				if (showMemoryManagerAllocations) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), allocationStr.c_str());

				lastAddress = currentAddress;
				currentAddress = memoryManagerAllocationIt->first;
				memoryManagerAllocationIt++;
			}
			else if (minDistance == distanceToPoolAllocation)
			{
				std::string allocationStr =
					poolAllocationIt->second.tag + "\n"
					"Address:   " + N2HexStr(poolAllocationIt->first) + "\n"
					"Size: " + std::to_string(poolAllocationIt->second.sizeInBytes) + "bytes\n\n";
				if (showPoolAllocations) ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), allocationStr.c_str());

				lastAddress = currentAddress;
				currentAddress = poolAllocationIt->first;
				poolAllocationIt++;
			}
			else if (minDistance == distanceToStackAllocation)
			{
				std::string allocationStr =
					stackAllocationIt->second.tag + "\n"
					"Address:   " + N2HexStr(stackAllocationIt->first) + "\n"
					"Size: " + std::to_string(stackAllocationIt->second.sizeInBytes) + "bytes\n\n";
				if (showStackAllocations) ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), allocationStr.c_str());

				lastAddress = currentAddress;
				currentAddress = stackAllocationIt->first;
				stackAllocationIt++;
			}
			else if (minDistance == distanceToFreeEntry)
			{
				std::string freeEntryStr =
					"Free Memory Block\n"
					"Address:   " + N2HexStr((size_t)pCurrentFreeEntry) + "\n"
					"Size: " + std::to_string(pCurrentFreeEntry->sizeInBytes) + "bytes\n"
					"Next Free: " + N2HexStr((size_t)pCurrentFreeEntry->pNext) + "\n\n";
				if (showMemoryManagerFreeBlock) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), freeEntryStr.c_str());

				lastAddress = currentAddress;
				currentAddress = (size_t)pCurrentFreeEntry;
				pCurrentFreeEntry = pCurrentFreeEntry->pNext;
			}
			else
			{
				assert(false);
			}
		}

		ImGui::TreePop();
	}
}
#endif

#ifdef SHOW_GRAPHS
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
			average /= valueCount;
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
#endif
#ifdef TEST_STACK_ALLOCATOR
void RunTest()
{
#ifdef MULTI_THREADED
#ifdef SHOW_GRAPHS
	InitThread();
#endif
#ifndef COLLECT_PERFORMANCE_DATA
	while (!g_StopThreads)
	{
#else
	for(size_t i = 0; i < NUM_FRAMES_TO_COLLECT_OVER; i++)
	{
#endif
#endif
#if (defined(MULTI_THREADED) && defined(SHOW_GRAPHS)) || defined(COLLECT_PERFORMANCE_DATA)
		MeasureThreadPerf();
#endif
		
		//Allocate a bunch of objects
		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
#ifdef SIMULATE_WORKLOADS
			gContainerArr[i] = STACK_NEW("Dummy Struct " + std::to_string(i)) DummyStruct(
				randf(-1024.0f, 1024.0f),
				randf(-1024.0f, 1024.0f),
				randf(-1024.0f, 1024.0f),
				randf(0, 2 * PI),
				randf(0, 2 * PI),
				randf(0, 2 * PI));
#else
			gContainerArr[i] = STACK_NEW("Dummy Struct") DummyStruct(
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f);
#endif
		}

#ifdef SIMULATE_WORKLOADS
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
#endif
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
#endif
}
#elif defined(TEST_POOL_ALLOCATOR)
void RunTest()
{
#ifdef MULTI_THREADED 
#ifdef SHOW_GRAPHS
	InitThread();
#endif
#ifndef COLLECT_PERFORMANCE_DATA
	while (!g_StopThreads)
	{
#else
	for (size_t i = 0; i < NUM_FRAMES_TO_COLLECT_OVER; i++)
	{
#endif
#endif
#if (defined(MULTI_THREADED) && defined(SHOW_GRAPHS)) || defined(COLLECT_PERFORMANCE_DATA)
		MeasureThreadPerf();
#endif

		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
#ifdef SIMULATE_WORKLOADS
			if (gContainerArr[i] == nullptr)
			{
				if (randf() < CHANCE_OF_ALLOCATION)
				{
					gContainerArr[i] = POOL_NEW(DummyStruct, "Pool Allocation " + std::to_string(i)) DummyStruct(
						randf(-1024.0f, 1024.0f),
						randf(-1024.0f, 1024.0f),
						randf(-1024.0f, 1024.0f),
						randf(0, 2 * PI),
						randf(0, 2 * PI),
						randf(0, 2 * PI));
				}
			}
#else 
			gContainerArr[i] = POOL_NEW(DummyStruct, "Pool Allocation") DummyStruct(
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				0.0f);
#endif
		}

#ifdef SIMULATE_WORKLOADS
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
#endif

		for (int i = 0; i < NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD; i++)
		{
#ifdef SIMULATE_WORKLOADS
			if (gContainerArr[i] != nullptr)
			{
				if (randf() < CHANCE_OF_FREE)
				{
					POOL_DELETE(gContainerArr[i]);
					gContainerArr[i] = nullptr;
				}
			}
#else
			POOL_DELETE(gContainerArr[i]);
			gContainerArr[i] = nullptr;
#endif
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

#ifdef ENABLE_GRAPHICAL_TEST
#define NROFCIRCLES 100
#define CIRCLERADIUS 10.0f
#define CIRCLEDIAMETER CIRCLERADIUS * 2
std::array<sf::CircleShape*, NROFCIRCLES> gCircleShapesPoolArray;

void circlePoolTest()
{
	float yPos = 0.0f;
	float xPos = 0.0f;
	for (int i = 0; i < NROFCIRCLES; i++)
	{
		if (randf() < 0.3f)
		{
			if (gCircleShapesPoolArray[i] != nullptr)
			{
				POOL_DELETE(gCircleShapesPoolArray[i]);
				gCircleShapesPoolArray[i] = nullptr;
			}
			gCircleShapesPoolArray[i] = POOL_NEW(sf::CircleShape, "sf::CircleShape " + std::to_string(i)) sf::CircleShape(CIRCLERADIUS);
			gCircleShapesPoolArray[i]->setFillColor(sf::Color::Red);
			if (i % 10 == 0)
			{
				yPos += CIRCLEDIAMETER;
				xPos = 0;
			}
			gCircleShapesPoolArray[i]->setPosition(xPos * CIRCLEDIAMETER, yPos);
			xPos++;

		}

	}


	for (int i = 0; i < NROFCIRCLES; i++)
	{
		if (gCircleShapesPoolArray[i] != nullptr)
		{
			if (randf() < 0.3f)
			{
				POOL_DELETE(gCircleShapesPoolArray[i]);
				gCircleShapesPoolArray[i] = nullptr;
			}
		}

	}
}

std::array<sf::CircleShape*, NROFCIRCLES> gCircleShapesStackArray;
void CircleStackTest(sf::RenderWindow& window)
{
	//Allocate a bunch of objects
	float yPos = CIRCLEDIAMETER * (NROFCIRCLES / 10);
	float xPos = 0.0f;
	for (int i = 0; i < NROFCIRCLES; i++)
	{
		gCircleShapesStackArray[i] = STACK_NEW("sf::CircleShape " + std::to_string(i)) sf::CircleShape(CIRCLERADIUS);
		gCircleShapesStackArray[i]->setFillColor(sf::Color::Green);
		if (i % 10 == 0)
		{
			yPos += CIRCLEDIAMETER;
			xPos = 0;
		}
		gCircleShapesStackArray[i]->setPosition(xPos * CIRCLEDIAMETER, yPos);
		xPos++;
	}

	//draw and change color on random circles
	for (int i = 0; i < NROFCIRCLES; i++)
	{
		if (randf() < 0.3f)
		{
			gCircleShapesStackArray[i]->setFillColor(sf::Color::Blue);
		}
		window.draw(*gCircleShapesStackArray[i]);
	}
	
	//Call object destructors
	for (int i = 0; i < NROFCIRCLES; i++)
	{
		//Delete just to call destructors on object, actual memory is reused
		STACK_DELETE(gCircleShapesStackArray[i]);
		gCircleShapesStackArray[i] = nullptr;
	}
}

#endif

int main(int argc, const char* argv[])
{    
	MEMLEAKCHECK;

	//Start program
#ifndef COLLECT_PERFORMANCE_DATA
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
		
		window.clear(bgColor);

#ifdef ENABLE_GRAPHICAL_TEST
		circlePoolTest();
		for (auto i : gCircleShapesPoolArray)
		{
			if (i != nullptr)
			{
				window.draw(*i);
			}
		}		
		
		CircleStackTest(window);
#endif

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

#ifdef SHOW_ALLOCATIONS_DEBUG
			ImGuiPrintMemoryManagerAllocations();
#endif

			ImGui::NextColumn();
#ifdef SHOW_GRAPHS
			ImGuiDrawFrameTimeGraph(deltaTime);
#endif
		}
		ImGui::End();
		ImGui::SFML::Render(window);
		EndFrame();

		window.display();

	#ifdef USE_CUSTOM_ALLOCATOR
		stack_reset();
	#endif
	}

#if defined(TEST_STACK_ALLOCATOR) || defined(TEST_POOL_ALLOCATOR)
	StopTest();
#endif

	ImGui::SFML::Shutdown();
#else
#if defined(TEST_STACK_ALLOCATOR) || defined(TEST_POOL_ALLOCATOR)
	for (size_t i = 0; i < NUM_FRAMES_TO_COLLECT_OVER; i++)
	{
		StartTest();

#ifdef USE_CUSTOM_ALLOCATOR
		stack_reset();
#endif
	}

	StopTest();

#ifndef MULTI_THREADED
	g_ThreadFrameSums[std::this_thread::get_id()] = threadPerformanceData.frameTimeSum;
#endif

	std::stringstream fileName;
	fileName << "Results/";

#ifdef TEST_POOL_ALLOCATOR
	fileName << "Pool ";
#endif
#ifdef TEST_STACK_ALLOCATOR
	fileName << "Stack ";
#endif
#ifdef MULTI_THREADED
	fileName << "MT ";
#endif
#ifdef USE_CUSTOM_ALLOCATOR
	fileName << "Custom Allocator ";
#else
	fileName << "Malloc ";
#endif

	fileName << NUMBER_OF_OBJECTS_IN_TEST_PER_THREAD << " ObjPerThread";
	fileName << ".txt";
	
	std::ofstream file;
	file.open(fileName.str(), std::ios::out | std::ios::trunc);

	for (auto it : g_ThreadFrameSums)
	{
		size_t totalMicroSeconds = it.second;
		float averageMicroSeconds = (float)totalMicroSeconds / NUM_FRAMES_TO_COLLECT_OVER;
		std::cout << averageMicroSeconds << std::endl;
		file << averageMicroSeconds << "|" << std::endl;
	}

	file.close();
#endif
#endif
	return 0; 
}
