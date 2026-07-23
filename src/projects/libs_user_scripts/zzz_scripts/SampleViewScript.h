#pragma once

#include <public/core/userscripts/base_script/ViewScript.h>

namespace zzz_scripts
{
	class SampleViewScript : public zzz::script::ViewScript
	{
	public:
		SampleViewScript() = default;
		~SampleViewScript() override = default;

	protected:
		void OnBindEvents() override;
	};
}
