#include "SampleObjectScript.h"
#include "core/logger/logger.h"

Z_SET_LOG_CATEGORY(::zzz::core::ScriptObject);

void SampleObjectScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::core::Time& time) { OnUpdate(time); });
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

void SampleObjectScript::OnUpdate([[maybe_unused]] const zzz::core::Time& time)
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
