#include "GlobalGameScript_001.h"
#include <common/Macroses.h>

void GlobalGameScript_001::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& time) { OnUpdate(time); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void GlobalGameScript_001::OnInit()
{
	DOut("[GlobalGameScript_001] - OnInit");
}

void GlobalGameScript_001::OnStart()
{
	DOut("[GlobalGameScript_001] - OnStart");
}

void GlobalGameScript_001::OnEnable()
{
	DOut("[GlobalGameScript_001] - OnEnable");
}

void GlobalGameScript_001::OnDisable()
{
	DOut("[GlobalGameScript_001] - OnDisable");
}

void GlobalGameScript_001::OnUpdate(const zzz::engine::Time& /*time*/)
{
	//static float timer = 0.0f;
	//timer += time.GetDeltaTime();
	//if (timer >= 2.0f)
	//{
	//	DOut("[GlobalGameScript_001] - OnUpdate (dt: {:.4f})", time.GetDeltaTime());
	//	timer = 0.0f;
	//}
}

void GlobalGameScript_001::OnDestroy()
{
	DOut("[GlobalGameScript_001] - OnDestroy");
}
