#pragma once
#include <GameScript.h>

class GlobalGameScript_002 : public zzz::script::GameScript
{
public:
	using GameScript::GameScript;
	const char* GetScriptTypeName() const override { return "GlobalGameScript_002"; }
	void OnBindEvents() override;

	void OnInit();
	void OnStart();
	void OnEnable();
	void OnDisable();
	void OnUpdate(float dt);
	void OnDestroy();
};
