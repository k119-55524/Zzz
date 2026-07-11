
#include <logger/logger.h>
#include <common/macroses.h>

#include "Test.hpp"

Test::Test()
	: zzz::script::GameScript()
{
}

void Test::OnBindEvents()
{
	DOut("Test script OnBindEvents called!");
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToStop([this]() { OnStop(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
}

void Test::OnStart()
{
	DOut("Test script started!");
}

void Test::OnStop()
{
	DOut("Test script stopped!");
}

void Test::OnUpdate(const zzz::engine::Time& time)
{
	static float timer = 0.0f;
	timer += time.GetDeltaTime();
	if (timer >= 1.0f)
	{
		DOut("Test script tick! (1 second)");
		timer -= 1.0f;
	}
}
