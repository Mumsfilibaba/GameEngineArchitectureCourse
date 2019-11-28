#include <iostream>
#include "GameAssign2.h"

int main()
{
	Game* pGame = new GameAssign2();
	pGame->Run();
	delete pGame;
	return 0;
}