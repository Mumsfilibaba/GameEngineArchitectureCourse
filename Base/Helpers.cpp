#include "Helpers.h"
#include <imgui.h>
#include <algorithm>
#include <mutex>
#include "SpinLock.h"
#include "MemoryManager.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
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

void ImGuiDrawMemoryProgressBar(int32_t used, int32_t available)
{
	float usedF = float(used) / float(std::max(available, 1));
	ImGui::ProgressBar(usedF, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	//Print megabytes
	float u = BTOMB(used);
	if (u > 0.1f) 
	{
		float a = BTOMB(available);
		ImGui::Text("(%.2f/%.2f) mb", u, a);
		return;
	}

	//Print kilobytes
	u = BTOKB(used);
	if (u > 0.1f)
	{
		float a = BTOKB(available);
		ImGui::Text("(%.2f/%.2f) kb", u, a);
	}
	else
	{
		//Print bytes
		ImGui::Text("(%.2f/%.2f) bytes", float(used), float(available));
	}
}

void ImGuiPrintMemoryManagerAllocations()
{
#ifdef SHOW_ALLOCATIONS_DEBUG
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

		auto memoryManagerAllocationIt = memoryManagerAllocations.begin();
		auto poolAllocationIt = poolAllocations.begin();
		auto stackAllocationIt = stackAllocations.begin();
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
#endif
}

void ImGuiDrawFrameTimeGraph(const sf::Time& dt)
{
#ifdef SHOW_GRAPHS
	static sf::Int64 timer = dt.asMicroseconds();
	timer += dt.asMicroseconds();

	static int fps = 0;
	static int currentFps = 0;
	currentFps++;
	if (timer >= 1000000)
	{
		fps = currentFps;
		currentFps = 0;
		timer = 0;
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
#endif
}

void ThreadSafePrintf(const char* pFormat, ...)
{
	static SpinLock printLock;
	std::scoped_lock<SpinLock> lock(printLock);

	va_list args;
	va_start(args, pFormat);
	vprintf(pFormat, args);
	va_end(args);
}

double FastAtof(const char* const str, int32_t& length)
{
	const char* iter = str;
	bool negative = false;
	double integer = 0.0;
	double decimal = 0.0;

	//Set length to zero
	length = 0;

	//Check sign
	if ((*iter) == '-')
	{
		negative = true;
		iter++;
		length++;
	}

	//Get "Interger"-part
	while ((*iter) > '/' && (*iter) < ':')
	{
		integer *= 10.0;
		integer += (*iter) - '0';

		length++;
		iter++;
	}

	//Get part after '.'
	if ((*iter) == '.')
	{
		iter++;
		length++;

		double base = 0.1;
		while ((*iter) > '/' && (*iter) < ':')
		{
			decimal += ((*iter) - '0') * base;
			base /= 10.0;

			length++;
			iter++;
		}
	}

	//Set sign
	if (negative)
		return -(integer + decimal);

	return integer + decimal;
}

int32_t FastAtoi(const char* const str, int32_t& length)
{
	const char* iter = str;
	int32_t integer = 0;
	bool negative = false;

	//Set length to zero
	length = 0;

	//Get sign
	if ((*iter) == '-')
	{
		length++;
		iter++;
		negative = true;
	}

	//Get Interger
	while ((*iter) > '/' && (*iter) < ':')
	{
		integer *= 10;
		integer += (*iter) - '0';

		length++;
		iter++;
	}

	if (negative)
		return -integer;

	return integer;
}