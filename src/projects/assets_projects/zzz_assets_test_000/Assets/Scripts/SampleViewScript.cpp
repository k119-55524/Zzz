#include "SampleViewScript.h"
#include "core/logger/logger.h"

Z_SET_LOG_CATEGORY(::zzz::core::ScriptView);

void SampleViewScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::core::Time& time) { OnUpdate(time); });
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

void SampleViewScript::OnUpdate([[maybe_unused]] const zzz::core::Time& time)
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
