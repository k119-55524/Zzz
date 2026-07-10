#pragma once
#include <GameScript.h>

class Test : public zzz::script::GameScript
{
	public:
		Test();
		virtual ~Test() = default;

		const char* GetScriptTypeName() const override { return "Test"; }
		void OnBindEvents() override;

		void OnStart();
		void OnUpdate(const zzz::engine::Time& time);

		int i;
};
