#pragma once

#include <public/core/userscripts/base_script/Script.h>

namespace zzz_scripts
{
	class SampleObjectScript : public zzz::script::Script
	{
	public:
		explicit SampleObjectScript(zzz::GameObject* owner);
		~SampleObjectScript() override = default;

	protected:
		void OnBindEvents() override;
	};
}
