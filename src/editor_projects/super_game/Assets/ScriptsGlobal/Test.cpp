#include "Test.hpp"

Test::Test()
	: zzz::script::GameScript()
{
}

void Test::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
}

void Test::OnStart()
{
}

void Test::OnUpdate(const zzz::engine::Time& time)
{
}
