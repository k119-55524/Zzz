
#include <common/Macroses.h>

#include "GlobalGameScript_002.h"

void GlobalGameScript_002::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void GlobalGameScript_002::OnInit()
{
	DOut("[GlobalGameScript_002] - OnInit");
}

void GlobalGameScript_002::OnStart()
{
	DOut("[GlobalGameScript_002] - OnStart");
}

void GlobalGameScript_002::OnEnable()
{
	DOut("[GlobalGameScript_002] - OnEnable");
}

void GlobalGameScript_002::OnDisable()
{
	DOut("[GlobalGameScript_002] - OnDisable");
}

void GlobalGameScript_002::OnUpdate([[maybe_unused]] const zzz::engine::Time& time)
{
	//static float timer = 0.0f;
	//timer += time.GetDeltaTime();
	//if (timer >= 2.0f)
	//{
	//	DOut("[GlobalGameScript_002] - OnUpdate (dt: {:.4f})", time.GetDeltaTime());
	//	timer = 0.0f;
	//}
}

void GlobalGameScript_002::OnDestroy()
{
	DOut("[GlobalGameScript_002] - OnDestroy");
}
