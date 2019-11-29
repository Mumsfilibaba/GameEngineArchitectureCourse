#include "Debugger.h"
#include "PoolAllocator.h"
#include "MemoryManager.h"
#include "StackAllocator.h"
#include <imgui.h>
#include <algorithm>

bool Debugger::s_DebugState = false;

void Debugger::DrawDebugUI(const sf::Time& deltatime)
{
	//Draw debug window
	if (s_DebugState)
	{
		if (ImGui::Begin("Debug Window", NULL, ImGuiWindowFlags_NoCollapse))
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
			ImGuiDrawFrameTimeGraph(deltatime);
		}
		ImGui::End();
	}
}

void Debugger::SetDebugState(bool debug)
{
	s_DebugState = debug;
}

bool Debugger::GetDebugState()
{
	return s_DebugState;
}
