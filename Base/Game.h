#pragma once
#include <SFML/System/Time.hpp>

class Game
{
public:
	Game() = default;
	~Game() = default;

	virtual void Init();
	virtual void Run();
	virtual void Update(sf::Time deltaTime);
	virtual void Render();
private:
};