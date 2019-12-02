#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "MemoryManager.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"
#include "Mesh.h"
#include "Camera.h"

class Game
{
public:
	Game() = default;
	~Game() = default;

	virtual void Init() {}
	virtual void Update(const sf::Time&) {}
	virtual void Render() {}
	virtual void RenderImGui() {}
	virtual void Release() {}
	void Run();

	inline void* operator new(size_t size)
	{
		MemoryManager& memorymanager = MemoryManager::GetInstance();
		return memorymanager.Allocate(size, 1, "Game Instance");
	}

	inline void operator delete(void* ptr)
	{
		MemoryManager& memorymanager = MemoryManager::GetInstance();
		memorymanager.Free(ptr);
	}
private:
	void InternalInit();
	void InternalUpdate(const sf::Time& deltatime);
	void InternalRender(const sf::Time& deltatime);
	void InternalRenderImGui(const sf::Time& deltatime);
	void InternalRelease();
protected:
	sf::RenderWindow* m_pRenderWindow;
	Camera m_Camera;
};
