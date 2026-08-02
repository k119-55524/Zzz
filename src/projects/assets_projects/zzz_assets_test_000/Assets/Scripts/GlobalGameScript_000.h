#pragma once
#include <GameScript.h>

class GlobalGameScript_000 : public zzz::script::GameScript
{
public:
	using GameScript::GameScript;
	const char* GetScriptTypeName() const override { return "GlobalGameScript_000"; }
	void OnBindEvents() override;

	void OnInit();
	void OnStart();
	void OnEnable();
	void OnDisable();
	void OnUpdate(const zzz::engine::Time& time);
	void OnDestroy();
};
