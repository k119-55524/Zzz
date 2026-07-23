#include "SampleViewScript.h"

namespace zzz_scripts
{
	void SampleViewScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте вью
		});

		SubscribeToUpdate([this](float dt)
		{
			// Логика каждого кадра вью
		});
	}
}
