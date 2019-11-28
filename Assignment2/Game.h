#pragma once
#include <SFML/System/Time.hpp>

class Game
{
public:
	Game() = default;
	~Game() = default;

	void Init();
	void Run();
	void Update(sf::Time deltaTime);
	void Render();
private:
};