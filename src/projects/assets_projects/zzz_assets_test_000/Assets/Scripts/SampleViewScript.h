#pragma once

#include <public/core/userscripts/base_script/ViewScript.h>

namespace zzz_assets_test_000
{
	class SampleViewScript : public zzz::script::ViewScript
	{
	public:
		SampleViewScript() = default;
		~SampleViewScript() override = default;

		const char* GetScriptTypeName() const override { return "SampleViewScript"; }

	protected:
		void OnBindEvents() override;
	};
}
