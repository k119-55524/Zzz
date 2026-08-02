#include <logger/logger.h>
#include "SampleObjectScript.h"

void SampleObjectScript::OnBindEvents()
{
	SubscribeToStart([this]() { OnStart(); });
	SubscribeToEnable([this]() { OnEnable(); });
	SubscribeToDisable([this]() { OnDisable(); });
	SubscribeToUpdate([this](float dt) { OnUpdate(dt); });
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

void SampleObjectScript::OnUpdate(float deltaTime)
{
	//static float timer = 0.0f;
	//timer += deltaTime;
	//if (timer >= 2.0f)
	//{
	//	DOut("[SampleObjectScript] - OnUpdate (dt: {:.4f})", deltaTime);
	//	timer = 0.0f;
	//}
}

void SampleObjectScript::OnDestroy()
{
	DOut("[SampleObjectScript] - OnDestroy");
}
