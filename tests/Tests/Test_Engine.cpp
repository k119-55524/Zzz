#include "Test_Engine.h"

Test_Engine::Test_Engine() :
	BaseTest("Test_Engine")
{
}

bool Test_Engine::Initialize()
{
	engine = make_unique<zEngine>();

	return engine != nullptr;
}

bool Test_Engine::Run()
{
	return true;
}

void Test_Engine::Shutdown()
{
	engine.reset(nullptr);
}