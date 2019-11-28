#pragma once
#include "Defines.h"
#include <string>
#include <SFML/System/Time.hpp>

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4505)		//Disable: "unreferenced local function has been removed"-warning
#endif

std::string N2HexStr(size_t w);

void ImGuiDrawMemoryProgressBar(int used, int available);
void ImGuiPrintMemoryManagerAllocations();
void ImGuiDrawFrameTimeGraph(const sf::Time& deltatime);

void ThreadSafePrintf(const char* pFormat, ...);

static float randf()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

static float randf(float min, float max)
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX)* (max - min) + min;
}