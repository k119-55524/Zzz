#pragma once

#include <public/core/userscripts/base_script/SceneScript.h>

namespace zzz_scripts
{
	class SampleSceneScript : public zzz::script::SceneScript
	{
	public:
		SampleSceneScript() = default;
		~SampleSceneScript() override = default;

	protected:
		void OnBindEvents() override;
	};
}
