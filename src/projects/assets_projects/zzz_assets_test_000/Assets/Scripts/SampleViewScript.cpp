
#include <logger/logger.h>

#include "SampleViewScript.h"

void SampleViewScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& t) { OnUpdate(t.GetDeltaTime()); });
	SubscribeToDestroy([this]() { OnDestroy(); });
}

void SampleViewScript::OnStart()
{
	DOut("[SampleViewScript] Событие: OnStart");
}

void SampleViewScript::OnEnable()
{
	DOut("[SampleViewScript] Событие: OnEnable");
}

void SampleViewScript::OnDisable()
{
	DOut("[SampleViewScript] Событие: OnDisable");
}

void SampleViewScript::OnUpdate(float deltaTime)
{
	// Отключаем логгер в каждом кадре, чтобы не забивать консоль, либо пишем периодически
	static float timer = 0.0f;
	timer += deltaTime;
	if (timer >= 2.0f)
	{
		DOut("[SampleViewScript] Событие: OnUpdate (dt: {})", deltaTime);
		timer = 0.0f;
	}
}

void SampleViewScript::OnDestroy()
{
	DOut("[SampleViewScript] Событие: OnDestroy");
}
