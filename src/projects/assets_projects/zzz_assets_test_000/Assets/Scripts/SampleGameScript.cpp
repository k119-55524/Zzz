
#include "SampleGameScript.h"
#include <common/macroses.h>

void SampleGameScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](const zzz::engine::Time& t) { OnUpdate(t.GetDeltaTime()); });
	SubscribeToDestroy([this]() { OnDestroy(); });

	OnInit();
}

void SampleGameScript::OnInit()
{
	DOut("[SampleGameScript] OnInit вызван.");
}

void SampleGameScript::OnStart()
{
	DOut("[SampleGameScript] OnStart получен!");
}

void SampleGameScript::OnEnable()
{
	DOut("[SampleGameScript] OnEnable получен!");
}

void SampleGameScript::OnDisable()
{
	DOut("[SampleGameScript] OnDisable получен!");
}

void SampleGameScript::OnUpdate(float /*dt*/)
{
	//DOut("[SampleGameScript] OnUpdate кадр, dt: {:.4f} с", dt);
}

void SampleGameScript::OnDestroy()
{
	DOut("[SampleGameScript] OnDestroy получен!");
}
