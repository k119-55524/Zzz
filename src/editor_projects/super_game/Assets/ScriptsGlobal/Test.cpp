#include "Test.hpp"
#include <logger/logger.h>
#include <common/macroses.h>
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
	DOut("Test script started!");
}

void Test::OnUpdate(const zzz::engine::Time& time)
{
}
