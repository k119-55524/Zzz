#include "GlobalGameScript_002.h"
#include <common/Macroses.h>

void GlobalGameScript_002::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& t) { OnUpdate(t.GetDeltaTime()); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void GlobalGameScript_002::OnInit()
{
	DOut("[GlobalGameScript_002] OnInit вызван.");
}

void GlobalGameScript_002::OnStart()
{
	DOut("[GlobalGameScript_002] OnStart получен!");
}

void GlobalGameScript_002::OnEnable()
{
	DOut("[GlobalGameScript_002] OnEnable получен!");
}

void GlobalGameScript_002::OnDisable()
{
	DOut("[GlobalGameScript_002] OnDisable получен!");
}

void GlobalGameScript_002::OnUpdate(float /*dt*/)
{
}

void GlobalGameScript_002::OnDestroy()
{
	DOut("[GlobalGameScript_002] OnDestroy получен!");
}
