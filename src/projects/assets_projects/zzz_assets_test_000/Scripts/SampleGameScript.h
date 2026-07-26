#pragma once

#include <public/core/userscripts/base_script/GameScript.h>

namespace zzz_assets_test_000
{
	class SampleGameScript : public zzz::script::GameScript
	{
	public:
		SampleGameScript() = default;
		~SampleGameScript() override = default;

		const char* GetScriptTypeName() const override { return "SampleGameScript"; }

	protected:
		void OnBindEvents() override;
	};
}
