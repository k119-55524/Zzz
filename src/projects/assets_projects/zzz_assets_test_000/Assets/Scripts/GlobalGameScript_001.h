#pragma once
#include <GameScript.h>

class GlobalGameScript_001 : public zzz::core::GameScript
{
public:
	using GameScript::GameScript;
	const char* GetScriptTypeName() const override { return "GlobalGameScript_001"; }
	void OnBindEvents() override;

	void OnInit();
	void OnStart();
	void OnEnable();
	void OnDisable();
	void OnUpdate(const zzz::core::Time& time);
	void OnDestroy();
};
