#include "SampleSceneScript.h"

namespace zzz_assets_test_000
{
	void SampleSceneScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте сцены
		});

		SubscribeToUpdate([this](const zzz::engine::Time& time)
		{
			// Логика каждого кадра сцены
		});
	}
}
