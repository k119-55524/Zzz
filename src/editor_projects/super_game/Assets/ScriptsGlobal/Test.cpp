#include "Test.hpp"

Test::Test()
	: zzz::script::GameScript()
{
}

void Test::Init(std::shared_ptr<zzz::engine::ProjectEventBus> bus)
{
	bus->OnStart.Subscribe(shared_from_this(), [this]() { OnStart(); });
	bus->OnUpdate.Subscribe(shared_from_this(), [this](const zzz::engine::Time& time) { OnUpdate(time); });
}

void Test::OnStart()
{
}

void Test::OnUpdate(const zzz::engine::Time& time)
{
}
