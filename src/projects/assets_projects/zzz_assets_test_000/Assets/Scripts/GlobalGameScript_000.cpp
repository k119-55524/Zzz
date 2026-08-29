#include "GlobalGameScript_000.h"
#include <core/Core.h>

Z_SET_LOG_CATEGORY(::zzz::core::ScriptGame);

void GlobalGameScript_000::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::core::Time& time) { OnUpdate(time); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void GlobalGameScript_000::OnInit()
{
	DOut("[GlobalGameScript_000] - OnInit");
}

void GlobalGameScript_000::OnStart()
{
	DOut("[GlobalGameScript_000] - OnStart");
}

void GlobalGameScript_000::OnEnable()
{
	DOut("[GlobalGameScript_000] - OnEnable");
}

void GlobalGameScript_000::OnDisable()
{
	DOut("[GlobalGameScript_000] - OnDisable");
}

void GlobalGameScript_000::OnUpdate([[maybe_unused]] const zzz::core::Time& time)
{
	//static float timer = 0.0f;
	//timer += time.GetDeltaTime();
	//if (timer >= 2.0f)
	//{
	//	DOut("[GlobalGameScript_000] - OnUpdate (dt: {:.8f})", time.GetDeltaTime());
	//	timer = 0.0f;
	//}
}

void GlobalGameScript_000::OnDestroy()
{
	DOut("[GlobalGameScript_000] - OnDestroy");
}
