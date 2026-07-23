#include "SampleSceneScript.h"

namespace zzz_scripts
{
	void SampleSceneScript::OnBindEvents()
	{
		SubscribeToStart([this]()
		{
			// Логика при старте сцены
		});

		SubscribeToUpdate([this](float dt)
		{
			// Логика каждого кадра сцены
		});
	}
}
