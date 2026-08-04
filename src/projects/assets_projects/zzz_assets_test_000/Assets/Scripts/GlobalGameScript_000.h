#pragma once
#include <core/Core.h>

class GlobalGameScript_000 : public zzz::core::GameScript
{
public:
	using GameScript::GameScript;
	const char* GetScriptTypeName() const override { return "GlobalGameScript_000"; }
	void OnBindEvents() override;

	void OnInit();
	void OnStart();
	void OnEnable();
	void OnDisable();
	void OnUpdate(const zzz::core::Time& time);
	void OnDestroy();
};
