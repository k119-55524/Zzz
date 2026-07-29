#pragma once
#include <GameScript.h>

class SampleGameScript : public zzz::script::GameScript
{
public:
	using GameScript::GameScript;
	const char* GetScriptTypeName() const override { return "SampleGameScript"; }
	void OnBindEvents() override;

	void OnInit();
	void OnStart();
	void OnEnable();
	void OnDisable();
	void OnUpdate(float dt);
	void OnDestroy();
};
