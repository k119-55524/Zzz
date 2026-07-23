#pragma once

#include <public/core/userscripts/base_script/GameScript.h>

namespace zzz_scripts
{
	class SampleGameScript : public zzz::script::GameScript
	{
	public:
		SampleGameScript() = default;
		~SampleGameScript() override = default;

	protected:
		void OnBindEvents() override;
	};
}
