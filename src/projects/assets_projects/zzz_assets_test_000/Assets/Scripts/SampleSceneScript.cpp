#include <logger/logger.h>
#include "SampleSceneScript.h"

void SampleSceneScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void SampleSceneScript::OnInit()
{
	DOut("[SampleSceneScript] - OnInit");
}

void SampleSceneScript::OnStart()
{
	DOut("[SampleSceneScript] - OnStart");
}

void SampleSceneScript::OnEnable()
{
	DOut("[SampleSceneScript] - OnEnable");
}

void SampleSceneScript::OnDisable()
{
	DOut("[SampleSceneScript] - OnDisable");
}

void SampleSceneScript::OnUpdate([[maybe_unused]] const zzz::engine::Time& time)
{
	//static float timer = 0.0f;
	//timer += time.GetDeltaTime();
	//if (timer >= 2.0f)
	//{
	//	DOut("[SampleSceneScript] - OnUpdate (dt: {:.4f})", time.GetDeltaTime());
	//	timer = 0.0f;
	//}
}

void SampleSceneScript::OnDestroy()
{
	DOut("[SampleSceneScript] - OnDestroy");
}
