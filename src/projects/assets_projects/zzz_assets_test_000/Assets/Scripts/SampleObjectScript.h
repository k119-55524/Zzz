#pragma once

#include <public/core/userscripts/base_script/Script.h>

namespace zzz_assets_test_000
{
	class SampleObjectScript : public zzz::script::Script
	{
	public:
		explicit SampleObjectScript(zzz::GameObject* owner);
		~SampleObjectScript() override = default;

		const char* GetScriptTypeName() const override { return "SampleObjectScript"; }

	protected:
		void OnBindEvents() override;
	};
}
