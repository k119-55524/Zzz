#include "SampleObjectScript.h"

namespace zzz_scripts
{
	SampleObjectScript::SampleObjectScript(zzz::GameObject* owner)
		: Script(owner)
	{
	}

	void SampleObjectScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте GameObject
		});

		SubscribeToUpdate([this](float dt)
		{
			// Логика каждого кадра GameObject
		});
	}
}
