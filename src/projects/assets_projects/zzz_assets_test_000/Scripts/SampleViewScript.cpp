#include "SampleViewScript.h"

namespace zzz_assets_test_000
{
	void SampleViewScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте вью
		});

		SubscribeToUpdate([this](const zzz::engine::Time& time)
		{
			// Логика каждого кадра вью
		});
	}
}
