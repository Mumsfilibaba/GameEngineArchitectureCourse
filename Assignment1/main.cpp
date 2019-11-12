#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>
#include <imgui-SFML.h>


class PoolAllocator
{
private:
	struct Block
	{
		Block* pNext		= nullptr;
		Block* pPrevious	= nullptr;
	};

public:
	inline PoolAllocator(int startSize, int blockSize)
		: m_pMemory(malloc(startSize)),
		m_pFreeListHead(nullptr),
		m_SizeInBytes(startSize),
		m_BlockSize(blockSize)
	{
		assert(startSize % blockSize == 0);
		assert(blockSize >= sizeof(Block));
		
		Block* pOld		= nullptr;
		Block* pCurrent = (Block*)m_pMemory;
		m_pFreeListHead = pCurrent;

		//Init blocks
		int blockCount = startSize / blockSize;
		for (int i = 0; i < blockCount; i++)
		{
			pCurrent->pNext = (Block*)(((char*)pCurrent) + blockSize); //HACKING;
			pCurrent->pPrevious = pOld;

			pOld	 = pCurrent;
			pCurrent = pCurrent->pNext;
		}

		//Set the last valid ptr's next to null
		pOld->pNext = nullptr;
	}

	
	inline ~PoolAllocator()
	{
		if (m_pMemory)
		{
			free(m_pMemory);
			m_pMemory = nullptr;
		}
	}


	inline void* AllocateBlock()
	{
		Block* pBlock = m_pFreeListHead;
		m_pFreeListHead = m_pFreeListHead->pNext;
		return (void*)pBlock;
	}


	template<typename T, typename... Args>
	inline T* MakeNew(Args&& ... args)
	{
		assert(m_BlockSize >= sizeof(T));
		return new(AllocateBlock()) T(std::forward<Args>(args) ...);
	}
private:
	void* m_pMemory;
	Block* m_pFreeListHead;
	int m_SizeInBytes;
	int m_BlockSize;
};


int main(int argc, const char* argv[])
{
	PoolAllocator pAllocator(4096, 16);
	int* pInt1 = pAllocator.MakeNew<int>(1);
	int* pInt2 = pAllocator.MakeNew<int>(2);
	int* pInt3 = pAllocator.MakeNew<int>(3);
	int* pInt4 = pAllocator.MakeNew<int>(4);
	int* pInt5 = pAllocator.MakeNew<int>(5);
	int* pInt6 = pAllocator.MakeNew<int>(6);


    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Engine Architecture");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    
    std::cout << "Hello World" << std::endl;
    
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

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
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        
		ImGui::ShowTestWindow();
        
		ImGui::Begin("Hello, world!");
		ImGui::Button("Look at this pretty button");
		ImGui::End();

		window.clear();
		window.draw(shape);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
    return 0; 
}
