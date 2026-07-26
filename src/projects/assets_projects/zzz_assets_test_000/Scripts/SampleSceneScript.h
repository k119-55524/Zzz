#pragma once

#include <public/core/userscripts/base_script/SceneScript.h>

namespace zzz_assets_test_000
{
	class SampleSceneScript : public zzz::script::SceneScript
	{
	public:
		SampleSceneScript() = default;
		~SampleSceneScript() override = default;

		const char* GetScriptTypeName() const override { return "SampleSceneScript"; }

	protected:
		void OnBindEvents() override;
	};
}
