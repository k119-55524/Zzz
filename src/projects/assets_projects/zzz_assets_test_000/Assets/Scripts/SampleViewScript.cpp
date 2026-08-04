
#include <logger/logger.h>

#include "SampleViewScript.h"

void SampleViewScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void SampleViewScript::OnInit()
{
	DOut("[SampleViewScript] - OnInit");
}

void SampleViewScript::OnStart()
{
	DOut("[SampleViewScript] - OnStart");
}

void SampleViewScript::OnEnable()
{
	DOut("[SampleViewScript] - OnEnable");
}

void SampleViewScript::OnDisable()
{
	DOut("[SampleViewScript] - OnDisable");
}

void SampleViewScript::OnUpdate([[maybe_unused]] const zzz::engine::Time& time)
{
	//static float timer = 0.0f;
	//timer += time.GetDeltaTime();
	//if (timer >= 2.0f)
	//{
	//	DOut("[SampleViewScript] - OnUpdate (dt: {:.12f})", time.GetDeltaTime());
	//	timer = 0.0f;
	//}
}

void SampleViewScript::OnDestroy()
{
	DOut("[SampleViewScript] - OnDestroy");
}
