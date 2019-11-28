#include "Helpers.h"

class Debugger
{
public:
	//Delete all these functions since we are a static class
	Debugger() = delete;
	Debugger(const Debugger&) = delete;
	Debugger(Debugger&&) = delete;
	~Debugger() = delete;
	Debugger& operator=(const Debugger&) = delete;
	Debugger& operator=(Debugger&&) = delete;

	//Actual functions
	static void DrawDebugUI(const sf::Time& deltatime);
	static void SetDebugState(bool debug);
	static bool GetDebugState();
private:
	static bool s_DebugState;
};