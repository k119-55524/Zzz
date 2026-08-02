#include "GlobalGameScript_000.h"
#include <common/Macroses.h>

void GlobalGameScript_000::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& t) { OnUpdate(t.GetDeltaTime()); });
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

void GlobalGameScript_000::OnUpdate(float /*deltaTime*/)
{
	//static float timer = 0.0f;
	//timer += deltaTime;
	//if (timer >= 2.0f)
	//{
	//	DOut("[GlobalGameScript_000] - OnUpdate (dt: {:.8f})", deltaTime);
	//	timer = 0.0f;
	//}
}

void GlobalGameScript_000::OnDestroy()
{
	DOut("[GlobalGameScript_000] - OnDestroy");
}
