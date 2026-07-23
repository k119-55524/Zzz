#include "SampleGameScript.h"

namespace zzz_scripts
{
	void SampleGameScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте проекта
		});

		SubscribeToUpdate([this](float dt)
		{
			// Логика каждого кадра проекта
		});
	}
}
