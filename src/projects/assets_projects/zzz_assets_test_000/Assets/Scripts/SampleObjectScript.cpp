#include <logger/logger.h>
#include "SampleObjectScript.h"

void SampleObjectScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void SampleObjectScript::OnInit()
{
	DOut("[SampleObjectScript] - OnInit");
}

void SampleObjectScript::OnStart()
{
	DOut("[SampleObjectScript] - OnStart");
}

void SampleObjectScript::OnEnable()
{
	DOut("[SampleObjectScript] - OnEnable");
}

void SampleObjectScript::OnDisable()
{
	DOut("[SampleObjectScript] - OnDisable");
}

void SampleObjectScript::OnUpdate(const zzz::engine::Time& /*time*/)
{
	//static float timer = 0.0f;
	//timer += time.GetDeltaTime();
	//if (timer >= 2.0f)
	//{
	//	DOut("[SampleObjectScript] - OnUpdate (dt: {:.4f})", time.GetDeltaTime());
	//	timer = 0.0f;
	//}
}

void SampleObjectScript::OnDestroy()
{
	DOut("[SampleObjectScript] - OnDestroy");
}
