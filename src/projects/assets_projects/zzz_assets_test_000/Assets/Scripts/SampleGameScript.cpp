#include "SampleGameScript.h"

namespace zzz_assets_test_000
{
	void SampleGameScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте проекта
		});

		SubscribeToUpdate([this](const zzz::engine::Time& time)
		{
			// Логика каждого кадра проекта
		});
	}
}
