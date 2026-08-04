#pragma once
#include <ViewScript.h>

class SampleViewScript : public zzz::core::ViewScript {
public:
	using ViewScript::ViewScript;
	const char* GetScriptTypeName() const override { return "SampleViewScript"; }

protected:
	void OnBindEvents() override;

private:
	void OnInit();
	void OnStart();
	void OnEnable();
	void OnDisable();
	void OnUpdate(const zzz::core::Time& time);
	void OnDestroy();
};
