#include "GlobalGameScript_000.h"
#include <common/macroses.h>

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
	DOut("[GlobalGameScript_000] OnInit вызван.");
}

void GlobalGameScript_000::OnStart()
{
	DOut("[GlobalGameScript_000] OnStart получен!");
}

void GlobalGameScript_000::OnEnable()
{
	DOut("[GlobalGameScript_000] OnEnable получен!");
}

void GlobalGameScript_000::OnDisable()
{
	DOut("[GlobalGameScript_000] OnDisable получен!");
}

void GlobalGameScript_000::OnUpdate(float /*dt*/)
{
}

void GlobalGameScript_000::OnDestroy()
{
	DOut("[GlobalGameScript_000] OnDestroy получен!");
}
