
#include <logger/logger.h>

#include "SampleViewScript.h"

void SampleViewScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& t) { OnUpdate(t.GetDeltaTime()); });
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

void SampleViewScript::OnUpdate(float /*deltaTime*/)
{
	//static float timer = 0.0f;
	//timer += deltaTime;
	//if (timer >= 2.0f)
	//{
	//	DOut("[SampleViewScript] - OnUpdate (dt: {:.12f})", deltaTime);
	//	timer = 0.0f;
	//}
}

void SampleViewScript::OnDestroy()
{
	DOut("[SampleViewScript] - OnDestroy");
}
