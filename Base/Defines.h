#pragma once

/*
 * Both SHOW_GRAPHS and COLLECT_PERFORMANCE_DATA should not be enabled at the same time
 * it's one or the other.
 */

#ifdef DEBUG
	#define SHOW_ALLOCATIONS_DEBUG
#endif
#define SHOW_GRAPHS 

//#define COLLECT_PERFORMANCE_DATA
//#define SIMULATE_WORKLOADS
//#define ENABLE_GRAPHICAL_TEST

#define PI 3.14159265359f
#define MB(mb) float(mb) * 1024.0f * 1024.0f
#define BTOKB(mb) float(mb) / (1024.0f)
#define BTOMB(mb) float(mb) / (1024.0f * 1024.0f)

#if defined(_WIN32)
	#include <crtdbg.h>
	#define MEMLEAKCHECK _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
	#define MEMLEAKCHECK
#endif