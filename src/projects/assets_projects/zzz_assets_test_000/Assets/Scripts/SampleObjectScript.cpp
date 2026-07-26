#include "SampleObjectScript.h"

namespace zzz_assets_test_000
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
