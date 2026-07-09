#pragma once

#include <common/templates/Event.h>
#include "public/core/EngineTime.h"

namespace zzz::engine
{
	class ProjectEventBus
	{
	public:
		zzz::engine::Event<> OnStart;
		zzz::engine::Event<> OnStop;
		zzz::engine::Event<const zzz::engine::Time&> OnUpdate;
	};

	class SceneEventBus
	{
	public:
		zzz::engine::Event<> OnStart;
		zzz::engine::Event<> OnStop;
		zzz::engine::Event<const zzz::engine::Time&> OnUpdate;
	};

	class GameObjectEventBus
	{
	public:
		zzz::engine::Event<> OnStart;
		zzz::engine::Event<> OnStop;
		zzz::engine::Event<float> OnUpdate;
	};
}
