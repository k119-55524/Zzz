#include "GlobalGameScript_001.h"
#include <common/Macroses.h>

void GlobalGameScript_001::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& t) { OnUpdate(t.GetDeltaTime()); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void GlobalGameScript_001::OnInit()
{
	DOut("[GlobalGameScript_001] OnInit вызван.");
}

void GlobalGameScript_001::OnStart()
{
	DOut("[GlobalGameScript_001] OnStart получен!");
}

void GlobalGameScript_001::OnEnable()
{
	DOut("[GlobalGameScript_001] OnEnable получен!");
}

void GlobalGameScript_001::OnDisable()
{
	DOut("[GlobalGameScript_001] OnDisable получен!");
}

void GlobalGameScript_001::OnUpdate(float /*dt*/)
{
}

void GlobalGameScript_001::OnDestroy()
{
	DOut("[GlobalGameScript_001] OnDestroy получен!");
}
